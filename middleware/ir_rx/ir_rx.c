#include "ir_rx.h"

#include "bsp_ir_rx_cfg.h"

#include "ir_rx_drv.h"
#include "ir_rx_nec.h"

static uint16_t s_decode_scratch[BSP_IR_RX_MAX_PULSE_RING_CAP];

typedef struct
{
  ir_rx_frame_cb_t frame_cb;
  void *frame_ctx;
} ir_rx_app_binding_t;

static ir_rx_app_binding_t s_app_binding[BSP_IR_RX_COUNT];

static void ir_rx_on_driver_event(ir_rx_channel_id_t channel, ir_rx_event_t event)
{
  ir_rx_nec_frame_t nec_frame;
  size_t pulse_count;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  if (event == IR_RX_EVT_BURST_READY) {
    pulse_count = ir_rx_drv_read_buffered_pulses(
        channel, s_decode_scratch, BSP_IR_RX_MAX_PULSE_RING_CAP);
    if (pulse_count < 4U) {
      ir_rx_drv_flush_buffer(channel);
      return;
    }

    if (ir_rx_nec_decode(s_decode_scratch, pulse_count, &nec_frame) != 0) {
      ir_rx_drv_flush_buffer(channel);
      return;
    }

    ir_rx_drv_flush_buffer(channel);

    if (s_app_binding[channel].frame_cb != NULL) {
      s_app_binding[channel].frame_cb(channel, &nec_frame,
                                      s_app_binding[channel].frame_ctx);
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

  s_app_binding[channel].frame_cb = NULL;
  s_app_binding[channel].frame_ctx = NULL;

  ir_rx_drv_init(channel);
  ir_rx_drv_set_event_fn(channel, ir_rx_on_driver_event);
}

void ir_rx_init_all(void)
{
  for (ir_rx_channel_id_t ch = 0U; ch < BSP_IR_RX_COUNT; ++ch) {
    ir_rx_init(ch);
  }
}

void ir_rx_register_frame_callback(ir_rx_channel_id_t channel,
                                   ir_rx_frame_cb_t cb, void *ctx)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  s_app_binding[channel].frame_cb = cb;
  s_app_binding[channel].frame_ctx = ctx;
}
