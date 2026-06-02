#include "task_ir.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ir_rx.h"
#include "ir_tx.h"
#include "taskmanager.h"

#define IR_TASK_STACK_WORDS  384U
#define IR_TASK_PRIO         (tskIDLE_PRIORITY + 2U)

static void task_ir(void *arg)
{
  (void)arg;

  for (;;) {
    vTaskDelay(portMAX_DELAY);
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
