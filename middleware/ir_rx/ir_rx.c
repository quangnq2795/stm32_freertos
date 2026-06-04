#include "ir_rx.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ir_rx_drv.h"
#include "ir_rx_nec.h"
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

static int ir_rx_try_read_burst(ir_rx_channel_id_t channel, ir_rx_burst_t *out)
{
  if (channel >= BSP_IR_RX_COUNT || out == NULL) {
    return IR_RX_ERR_PARAM;
  }

  if (slot_queue_try_pop(&s_burst_queue[channel], out) != SLOT_QUEUE_OK) {
    return IR_RX_ERR_EMPTY;
  }

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
