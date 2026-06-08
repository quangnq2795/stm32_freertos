#include "task.cli.h"

#include "FreeRTOS.h"
#include "task.h"

#include "cli.h"
#include "log.h"
#include "taskmanager.h"

#define CLI_TASK_STACK_WORDS  512U
#define CLI_TASK_PRIO         (tskIDLE_PRIORITY + 1U)

static void cmd_help(int argc, char **argv);

static void cmd_help(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  log_printf("Commands:");
  log_printf("  help");
}

static void cli_task_init(void *ctx)
{
  (void)ctx;

  if (cli_init() != CLI_OK) {
    return;
  }

  (void)cli_register_command("help", cmd_help);
}

static void cli_task_uninit(void *ctx)
{
  (void)ctx;

  cli_uninit();
}

static void cli_task_handler(void *ctx)
{
  (void)ctx;

  cli_process();
}

void task_cli_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_CLI,
      .name = "cli",
      .ops = {
          .task_init = cli_task_init,
          .task_uninit = cli_task_uninit,
          .task_handler = cli_task_handler,
      },
      .stack_words = CLI_TASK_STACK_WORDS,
      .priority = CLI_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }

  s_started = 1U;
  (void)tm_init(&cfg);
}
