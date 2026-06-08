#include "ir.h"

#include "bsp_ir_tx_cfg.h"
#include "ir_rx.h"
#include "ir_tx.h"
#include "sys_msg.h"
#include "taskmanager.h"

static ir_rx_frame_cb_t s_rx_cb;

static void ir_handle_rx(const sys_msg_t *msg)
{
  ir_rx_nec_frame_t frame;
  ir_rx_channel_id_t channel = (ir_rx_channel_id_t)msg->u.arg.param1;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  while (ir_rx_try_read_nec(channel, &frame) != IR_RX_ERR_EMPTY) {
    if (s_rx_cb != NULL) {
      s_rx_cb(channel, &frame);
    }
  }
}

static void ir_handle_tx(const sys_msg_t *msg)
{
  ir_tx_channel_id_t channel = (ir_tx_channel_id_t)msg->u.arg.param1;

  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  if (msg->u.arg.param4 != 0U) {
    (void)ir_tx_send_nec_repeat(channel);
    return;
  }

  (void)ir_tx_send_nec(channel, (uint8_t)msg->u.arg.param2,
                       (uint8_t)msg->u.arg.param3);
}

int ir_init(void)
{
  ir_rx_init_all();
  ir_tx_init_all();
  return IR_OK;
}

void ir_uninit(void)
{
}

void ir_process(void)
{
  sys_msg_t msg;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      switch (msg.opcode) {
      case IR_OPCODE_RX:
        ir_handle_rx(&msg);
        break;
      case IR_OPCODE_TX:
        ir_handle_tx(&msg);
        break;
      default:
        break;
      }
    }
  }
}

int ir_register_rx_callback(ir_rx_frame_cb_t cb)
{
  if (cb == NULL) {
    return IR_ERR_PARAM;
  }

  s_rx_cb = cb;
  return IR_OK;
}
