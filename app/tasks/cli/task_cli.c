#include "FreeRTOS.h"

#include <string.h>

#include "cli_cmd.h"
#include "sys_msg.h"
#include "task.cli.h"
#include "taskmanager.h"
#include "uart.h"

#define CLI_OPCODE_UART_RX 0U

#ifndef CLI_UART_ID
#define CLI_UART_ID 0U
#endif

#define CLI_TASK_STACK_WORDS 512U
#define CLI_TASK_PRIO (tskIDLE_PRIORITY + 1U)

static void cli_process_line(char *line, size_t line_len);
static void cli_msg_handler(const sys_msg_t *msg);

void cli_print(const char *s)
{
  if (s == NULL) {
    return;
  }
  (void)uart_write(CLI_UART_ID, (const uint8_t *)s, strlen(s));
}

static void cmd_help(int argc, char **argv);

static const cli_cmd_entry_t s_builtin_cmds[] = {
    {"help", cmd_help},
    {0, 0},
};

static const cli_cmd_entry_t *const s_cmd_tables[] = {
    s_builtin_cmds,
};

static void cli_uart_evt_cb(uart_id_t id, uart_event_t evt)
{
  sys_msg_t msg = {0};
  BaseType_t hpw = pdFALSE;

  (void)id;

  if (evt != UART_EVENT_RX_AVAILABLE) {
    return;
  }

  msg.dst = (uint32_t)SYS_NODE_CLI;
  msg.opcode = CLI_OPCODE_UART_RX;
  if (tm_send_from_isr(SYS_NODE_CLI, &msg, &hpw) == TM_OK) {
    portYIELD_FROM_ISR(hpw);
  }
}

static void uart_rx_handler(void)
{
  static char line[64];
  static size_t line_len;

  while (uart_rx_available(CLI_UART_ID) > 0U) {
    uint8_t buf[32];
    size_t n = uart_read(CLI_UART_ID, buf, sizeof(buf));
    for (size_t i = 0; i < n; ++i) {
      uint8_t c = buf[i];
      if (c == '\r' || c == '\n') {
        if (line_len > 0U) {
          line[line_len] = '\0';
          cli_process_line(line, line_len);
        }
        line_len = 0U;
      } else if (line_len < (sizeof(line) - 1U)) {
        line[line_len++] = (char)c;
      } else {
        line_len = 0U;
      }
    }
  }
}

static void cli_msg_handler(const sys_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  if (msg->opcode == CLI_OPCODE_UART_RX) {
    uart_rx_handler();
    return;
  }
}

static void task_cli(void *argument)
{
  sys_msg_t msg;

  (void)argument;

  uart_init(CLI_UART_ID);
  uart_register_event_callback(CLI_UART_ID, cli_uart_evt_cb);

  for (;;) {
    tm_task_wait_noti();
    while (tm_recv(&msg) == TM_OK) {
      cli_msg_handler(&msg);
    }
  }
}

static void cmd_help(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  cli_print("Commands:\r\n");
  cli_print("  help\r\n");
}

static void cli_process_line(char *line, size_t line_len)
{
  (void)line_len;

  const int MAX_ARGS = 6;
  char *argv[MAX_ARGS];
  int argc = 0;

  char *p = line;
  while (*p != '\0' && argc < MAX_ARGS) {
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
    for (const cli_cmd_entry_t *e = s_cmd_tables[t]; e->name != 0; ++e) {
      if (strcmp(argv[0], e->name) == 0) {
        e->fn(argc, argv);
        return;
      }
    }
  }

  cli_print("Unknown command\r\n");
}

void task_cli_create(void)
{
  static uint8_t s_started;
  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_CLI,
      .name = "cli",
      .entry = task_cli,
      .stack_words = CLI_TASK_STACK_WORDS,
      .priority = CLI_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }
  s_started = 1U;
  (void)tm_init(&cfg);
}
