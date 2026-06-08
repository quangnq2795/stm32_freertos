#include "ir_tx.h"

#include "bsp_ir_tx_cfg.h"

#include "ir_tx_drv.h"
#include "ir_tx_nec.h"

static void ir_tx_on_driver_complete(ir_tx_channel_id_t channel,
                                     ir_tx_xmit_status_t status, void *ctx)
{
  (void)channel;
  (void)status;
  (void)ctx;
}

void ir_tx_init(ir_tx_channel_id_t channel)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  ir_tx_drv_init(channel);
  ir_tx_drv_set_complete_fn(channel, ir_tx_on_driver_complete, NULL);
}

void ir_tx_init_all(void)
{
  for (ir_tx_channel_id_t ch = 0U; ch < BSP_IR_TX_COUNT; ++ch) {
    ir_tx_init(ch);
  }
}

int ir_tx_send_nec(ir_tx_channel_id_t channel, uint8_t address, uint8_t command)
{
  ir_tx_wave_t waves[IR_TX_NEC_WAVE_CAP];
  size_t wave_count;

  if (channel >= BSP_IR_TX_COUNT) {
    return -1;
  }

  if (ir_tx_drv_is_busy(channel) != 0U) {
    return -1;
  }

  wave_count = ir_tx_nec_encode(address, command, waves, IR_TX_NEC_WAVE_CAP);
  if (wave_count == 0U) {
    return -1;
  }

  if (ir_tx_drv_start_waves(channel, waves, wave_count) != 0) {
    return -1;
  }
  return 0;
}

int ir_tx_send_nec_repeat(ir_tx_channel_id_t channel)
{
  ir_tx_wave_t waves[IR_TX_NEC_WAVE_CAP];
  size_t wave_count;

  if (channel >= BSP_IR_TX_COUNT) {
    return -1;
  }

  if (ir_tx_drv_is_busy(channel) != 0U) {
    return -1;
  }

  wave_count = ir_tx_nec_encode_repeat(waves, IR_TX_NEC_WAVE_CAP);
  if (wave_count == 0U) {
    return -1;
  }

  if (ir_tx_drv_start_waves(channel, waves, wave_count) != 0) {
    return -1;
  }
  return 0;
}
