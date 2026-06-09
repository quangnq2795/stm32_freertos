#include "ir_rx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ir_rx_drv.h"
#include "ir_rx_nec.h"
#include "log.h"
#include "slot_queue.h"
#include "sys_msg.h"
#include "taskmanager.h"

static ir_rx_burst_t s_burst_storage[BSP_IR_RX_COUNT][IR_RX_BURST_QUEUE_DEPTH];
static slot_queue_t s_burst_queue[BSP_IR_RX_COUNT];

static int ir_rx_burst_queue_push_from_isr(ir_rx_channel_id_t channel)
{
  ir_rx_burst_t *slot;
  size_t pulse_count;

  if (channel >= BSP_IR_RX_COUNT) {
    return IR_RX_ERR_PARAM;
  }

  slot = (ir_rx_burst_t *)slot_queue_acquire(&s_burst_queue[channel]);
  if (slot == NULL) {
    ir_rx_drv_flush_buffer(channel);
    return IR_RX_ERR_EMPTY;
  }

  pulse_count = ir_rx_drv_read_buffered_pulses(
      channel, slot->pulses, BSP_IR_RX_MAX_PULSE_RING_CAP);
  if (pulse_count == 0U) {
    return IR_RX_ERR_EMPTY;
  }

  slot->count = (uint16_t)pulse_count;
  slot_queue_release(&s_burst_queue[channel]);
  return IR_RX_OK;
}

static void ir_rx_on_driver_event(ir_rx_channel_id_t channel, ir_rx_event_t event)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  if (event == IR_RX_EVT_BURST_READY) {
    if (ir_rx_burst_queue_push_from_isr(channel) == IR_RX_OK) {
      sys_msg_t msg = {0};
      BaseType_t hpw = pdFALSE;

      msg.opcode = IR_OPCODE_RX;
      msg.u.arg.param1 = (uint32_t)channel;
      if (tm_send_from_isr(SYS_NODE_IR, &msg, &hpw) == TM_OK) {
        portYIELD_FROM_ISR(hpw);
      }
    }
  } else if (event == IR_RX_EVT_BUF_OVERFLOW) {
    ir_rx_drv_flush_buffer(channel);
  }
}

void ir_rx_init(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  slot_queue_init(&s_burst_queue[channel], s_burst_storage[channel],
                  (uint16_t)sizeof(ir_rx_burst_t),
                  (uint8_t)IR_RX_BURST_QUEUE_DEPTH);

  ir_rx_drv_init(channel);
  ir_rx_drv_set_event_fn(channel, ir_rx_on_driver_event);
}

void ir_rx_init_all(void)
{
  for (ir_rx_channel_id_t ch = 0U; ch < BSP_IR_RX_COUNT; ++ch) {
    ir_rx_init(ch);
  }
}

#define IR_RX_LOG_PULSES_PER_LINE  8U

static void ir_rx_log_pulse_widths(ir_rx_channel_id_t channel,
                                   const uint16_t *pulses, uint16_t count)
{
  uint16_t i = 0U;

  log_printf("IR RX ch=%u n=%u us", (unsigned int)channel, (unsigned int)count);

  while (i < count) {
    uint16_t n = count - i;

    if (n > IR_RX_LOG_PULSES_PER_LINE) {
      n = IR_RX_LOG_PULSES_PER_LINE;
    }

    switch (n) {
      case 8U:
        log_printf("  %u %u %u %u %u %u %u %u",
                   (unsigned int)pulses[i], (unsigned int)pulses[i + 1U],
                   (unsigned int)pulses[i + 2U], (unsigned int)pulses[i + 3U],
                   (unsigned int)pulses[i + 4U], (unsigned int)pulses[i + 5U],
                   (unsigned int)pulses[i + 6U], (unsigned int)pulses[i + 7U]);
        break;
      case 7U:
        log_printf("  %u %u %u %u %u %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U], (unsigned int)pulses[i + 2U],
                   (unsigned int)pulses[i + 3U], (unsigned int)pulses[i + 4U],
                   (unsigned int)pulses[i + 5U], (unsigned int)pulses[i + 6U]);
        break;
      case 6U:
        log_printf("  %u %u %u %u %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U], (unsigned int)pulses[i + 2U],
                   (unsigned int)pulses[i + 3U], (unsigned int)pulses[i + 4U],
                   (unsigned int)pulses[i + 5U]);
        break;
      case 5U:
        log_printf("  %u %u %u %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U], (unsigned int)pulses[i + 2U],
                   (unsigned int)pulses[i + 3U], (unsigned int)pulses[i + 4U]);
        break;
      case 4U:
        log_printf("  %u %u %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U], (unsigned int)pulses[i + 2U],
                   (unsigned int)pulses[i + 3U]);
        break;
      case 3U:
        log_printf("  %u %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U], (unsigned int)pulses[i + 2U]);
        break;
      case 2U:
        log_printf("  %u %u", (unsigned int)pulses[i],
                   (unsigned int)pulses[i + 1U]);
        break;
      default:
        log_printf("  %u", (unsigned int)pulses[i]);
        break;
    }

    i += n;
  }
}

static int ir_rx_try_read_burst(ir_rx_channel_id_t channel, ir_rx_burst_t *out)
{
  if (channel >= BSP_IR_RX_COUNT || out == NULL) {
    return IR_RX_ERR_PARAM;
  }

  if (slot_queue_try_pop(&s_burst_queue[channel], out) != SLOT_QUEUE_OK) {
    return IR_RX_ERR_EMPTY;
  }

  ir_rx_log_pulse_widths(channel, out->pulses, out->count);

  return IR_RX_OK;
}

int ir_rx_try_read_nec(ir_rx_channel_id_t channel, ir_rx_nec_frame_t *out)
{
  ir_rx_burst_t burst;

  if (out == NULL) {
    return IR_RX_ERR_PARAM;
  }

  if (ir_rx_try_read_burst(channel, &burst) != IR_RX_OK) {
    return IR_RX_ERR_EMPTY;
  }

  if (ir_rx_nec_decode(burst.pulses, burst.count, out) != 0) {
    return IR_RX_ERR_DECODE;
  }

  return IR_RX_OK;
}
