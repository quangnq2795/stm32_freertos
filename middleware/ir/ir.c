#include "ir.h"

#include "bsp_ir_tx_cfg.h"
#include "ir_rx.h"
#include "ir_tx.h"
#include "log.h"
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

  for (;;) {
    int rc = ir_rx_try_read_nec(channel, &frame);

    if (rc == IR_RX_ERR_EMPTY) {
      break;
    }
    if (rc == IR_RX_ERR_PARAM) {
      log_printf("IR RX ch=%u err=param", (unsigned int)channel);
      break;
    }
    if (rc == IR_RX_ERR_DECODE) {
      log_printf("IR RX ch=%u err=decode", (unsigned int)channel);
      continue;
    }
    if (rc != IR_RX_OK) {
      break;
    }

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

int ir_send_nec(ir_tx_channel_id_t channel,
                uint8_t address,
                uint8_t command,
                TickType_t timeout)
{
  sys_msg_t msg = {0};

  msg.opcode = IR_OPCODE_TX;
  msg.u.arg.param1 = (uint32_t)channel;
  msg.u.arg.param2 = (uint32_t)address;
  msg.u.arg.param3 = (uint32_t)command;
  msg.u.arg.param4 = 0U;

  if (tm_send(SYS_NODE_IR, &msg, timeout) != TM_OK) {
    return IR_ERR_SEND;
  }

  return IR_OK;
}

int ir_send_nec_repeat(ir_tx_channel_id_t channel, TickType_t timeout)
{
  sys_msg_t msg = {0};

  msg.opcode = IR_OPCODE_TX;
  msg.u.arg.param1 = (uint32_t)channel;
  msg.u.arg.param2 = 0U;
  msg.u.arg.param3 = 0U;
  msg.u.arg.param4 = 1U;

  if (tm_send(SYS_NODE_IR, &msg, timeout) != TM_OK) {
    return IR_ERR_SEND;
  }

  return IR_OK;
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
