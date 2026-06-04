#include "task_ir.h"

#include "FreeRTOS.h"
#include "task.h"

#include "bsp_ir_rx_cfg.h"
#include "ir_rx.h"
#include "ir_tx.h"
#include "sys_msg.h"
#include "taskmanager.h"

#define IR_TASK_STACK_WORDS  384U
#define IR_TASK_PRIO         (tskIDLE_PRIORITY + 2U)

static void ir_handle_burst_ready(const sys_msg_t *msg)
{
  ir_rx_nec_frame_t frame;

  if (msg->u.arg.param1 >= BSP_IR_RX_COUNT) {
    return;
  }

  while (ir_rx_try_read_nec((ir_rx_channel_id_t)msg->u.arg.param1, &frame) !=
         IR_RX_ERR_EMPTY) {
  }
}

static void ir_msg_handler(const sys_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  switch (msg->opcode) {
  case IR_RX_OPCODE_BURST_READY:
    ir_handle_burst_ready(msg);
    break;
  default:
    break;
  }
}

static void task_ir(void *arg)
{
  sys_msg_t msg;

  (void)arg;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      ir_msg_handler(&msg);
    }
  }
}

void task_ir_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_IR,
      .name = "ir",
      .entry = task_ir,
      .stack_words = IR_TASK_STACK_WORDS,
      .priority = IR_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }
  s_started = 1U;

  ir_rx_init_all();
  ir_tx_init_all();
  (void)tm_init(&cfg);
}
