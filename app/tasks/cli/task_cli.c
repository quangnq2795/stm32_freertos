#include "task.cli.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "cli_cmd.h"
#include "log.h"
#include "serial.h"
#include "sys_msg.h"
#include "taskmanager.h"

#ifndef CLI_SERIAL_RX
#define CLI_SERIAL_RX  SERIAL_PORT_1_RX
#endif

#ifndef CLI_UART_RX_CHUNK
#define CLI_UART_RX_CHUNK  32U
#endif

#define CLI_LINE_MAX  64U
#define CLI_TASK_STACK_WORDS  512U
#define CLI_TASK_PRIO         (tskIDLE_PRIORITY + 1U)

static serial_t s_cli_rx;
static uint8_t s_serial_ready;

static void cli_process_line(char *line, size_t line_len);


static void cli_serial_rx_isr(uart_id_t port, uart_event_t evt, void *ctx)
{
  sys_msg_t msg = {0};
  BaseType_t hpw = pdFALSE;

  (void)port;
  (void)ctx;

  if (evt != UART_EVENT_RX_AVAILABLE) {
    return;
  }

  msg.opcode = CLI_OPCODE_RX;
  if (tm_send_from_isr(SYS_NODE_CLI, &msg, &hpw) == TM_OK) {
    portYIELD_FROM_ISR(hpw);
  }
}

static int cli_serial_setup(void)
{
  static const serial_cfg_t rx_cfg = {
      .isr_fn = cli_serial_rx_isr,
      .ctx = NULL,
  };

  if (serial_register(CLI_SERIAL_RX, SERIAL_TYPE_RX, &rx_cfg, &s_cli_rx) !=
      SERIAL_OK) {
    return SERIAL_ERR_BUSY;
  }

  s_serial_ready = 1U;
  return SERIAL_OK;
}

static void cli_rx_handler(void)
{
  static char line[CLI_LINE_MAX];
  static size_t line_len;
  uint8_t buf[CLI_UART_RX_CHUNK];
  size_t n;

  while ((n = serial_read(&s_cli_rx, buf, sizeof(buf))) > 0U) {
    for (size_t i = 0U; i < n; ++i) {
      uint8_t c = buf[i];

      if (c == '\r' || c == '\n') {
        if (line_len > 0U) {
          line[line_len] = '\0';
          cli_process_line(line, line_len);
        }
        line_len = 0U;
      } else if (line_len < (CLI_LINE_MAX - 1U)) {
        line[line_len++] = (char)c;
      } else {
        line_len = 0U;
      }
    }
  }
}

static void cli_task_init(void *ctx)
{
  (void)ctx;

  if (cli_serial_setup() != SERIAL_OK) {
    return;
  }
}

static void cli_task_uninit(void *ctx)
{
  (void)ctx;

  s_serial_ready = 0U;
  serial_unregister(&s_cli_rx);
}

static void cli_task_handler(void *ctx)
{
  sys_msg_t msg;

  (void)ctx;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      if (msg.opcode == CLI_OPCODE_RX) {
        cli_rx_handler();
      }
    }
  }
}

static void cmd_help(int argc, char **argv);

static const cli_cmd_entry_t s_builtin_cmds[] = {
    {"help", cmd_help},
    {0, 0},
};

static const cli_cmd_entry_t *const s_cmd_tables[] = {
    s_builtin_cmds,
};

static void cmd_help(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  log_printf("Commands:");
  log_printf("  help");
}

static void cli_process_line(char *line, size_t line_len)
{
  const int max_args = 6;
  char *argv[max_args];
  int argc = 0;
  char *p = line;

  (void)line_len;

  while (*p != '\0' && argc < max_args) {
    while (*p == ' ' || *p == '\t') {
      ++p;
    }
    if (*p == '\0') {
      break;
    }
    argv[argc++] = p;
    while (*p != '\0' && *p != ' ' && *p != '\t') {
      ++p;
    }
    if (*p != '\0') {
      *p = '\0';
      ++p;
    }
  }

  if (argc == 0) {
    return;
  }

  for (size_t t = 0U; t < (sizeof(s_cmd_tables) / sizeof(s_cmd_tables[0])); ++t) {
    for (const cli_cmd_entry_t *e = s_cmd_tables[t]; e->name != NULL; ++e) {
      if (strcmp(argv[0], e->name) == 0) {
        e->fn(argc, argv);
        return;
      }
    }
  }

  log_printf("Unknown command");
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
