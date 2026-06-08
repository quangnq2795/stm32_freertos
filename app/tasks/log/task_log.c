#include "task_log.h"

#include "FreeRTOS.h"
#include "task.h"

#include "log.h"
#include "taskmanager.h"

#define LOG_TASK_STACK_WORDS  384U
#define LOG_TASK_PRIO         (tskIDLE_PRIORITY + 1U)

static void log_task_init(void *ctx)
{
  (void)ctx;

  if (log_init() != LOG_OK) {
    return;
  }

  log_printf("log task ready");
}

static void log_task_uninit(void *ctx)
{
  (void)ctx;

  log_uninit();
}

static void log_task_handler(void *ctx)
{
  (void)ctx;

  log_process();
}

void task_log_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_LOG,
      .name = "log",
      .ops = {
          .task_init = log_task_init,
          .task_uninit = log_task_uninit,
          .task_handler = log_task_handler,
      },
      .stack_words = LOG_TASK_STACK_WORDS,
      .priority = LOG_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }

  s_started = 1U;
  (void)tm_init(&cfg);
}
