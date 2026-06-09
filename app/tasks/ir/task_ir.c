#include "task_ir.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ir.h"
#include "log.h"
#include "taskmanager.h"

#define IR_TASK_STACK_WORDS  384U
#define IR_TASK_PRIO         (tskIDLE_PRIORITY + 2U)

static void ir_on_frame(ir_rx_channel_id_t channel, const ir_rx_nec_frame_t *frame)
{
  (void)channel;

  if (frame->is_repeat != 0U) {
    log_printf("IR repeat");
    return;
  }

  log_printf("IR addr=0x%02X cmd=0x%02X", frame->address, frame->command);
}

static void ir_task_init(void *ctx)
{
  (void)ctx;

  if (ir_init() != IR_OK) {
    return;
  }

  (void)ir_register_rx_callback(ir_on_frame);
}

static void ir_task_uninit(void *ctx)
{
  (void)ctx;

  ir_uninit();
}

static void ir_task_handler(void *ctx)
{
  (void)ctx;

  ir_process();
}

void task_ir_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_IR,
      .name = "ir",
      .ops = {
          .task_init = ir_task_init,
          .task_uninit = ir_task_uninit,
          .task_handler = ir_task_handler,
      },
      .stack_words = IR_TASK_STACK_WORDS,
      .priority = IR_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }

  s_started = 1U;
  (void)tm_init(&cfg);
}
