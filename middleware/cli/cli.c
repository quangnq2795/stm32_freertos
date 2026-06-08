#include "cli.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "log.h"
#include "serial.h"
#include "sys_msg.h"
#include "taskmanager.h"

#ifndef CLI_SERIAL_RX
#define CLI_SERIAL_RX  SERIAL_PORT_2_RX
#endif

#ifndef CLI_UART_RX_CHUNK
#define CLI_UART_RX_CHUNK  32U
#endif

#define CLI_LINE_MAX       64U
#define CLI_CMD_TABLE_MAX  16U

typedef struct
{
  const char *name;
  cli_cmd_fn_t fn;
} cli_cmd_entry_t;

static serial_t s_cli_rx;
static uint8_t s_serial_ready;

static cli_cmd_entry_t s_cmds[CLI_CMD_TABLE_MAX];
static size_t s_cmd_count;

static void cli_process_line(char *line, size_t line_len);
static void cli_serial_rx_isr(uart_id_t port, uart_event_t evt, void *ctx);

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

  for (size_t i = 0U; i < s_cmd_count; ++i) {
    if (strcmp(argv[0], s_cmds[i].name) == 0) {
      s_cmds[i].fn(argc, argv);
      return;
    }
  }

  log_printf("Unknown command");
}

int cli_init(void)
{
  static const serial_cfg_t rx_cfg = {
      .isr_fn = cli_serial_rx_isr,
      .ctx = NULL,
  };

  if (s_serial_ready != 0U) {
    return CLI_OK;
  }

  if (serial_register(CLI_SERIAL_RX, SERIAL_TYPE_RX, &rx_cfg, &s_cli_rx) !=
      SERIAL_OK) {
    return CLI_ERR_BUSY;
  }

  s_serial_ready = 1U;
  return CLI_OK;
}

void cli_uninit(void)
{
  if (s_serial_ready == 0U) {
    return;
  }

  s_serial_ready = 0U;
  serial_unregister(&s_cli_rx);
}

void cli_process(void)
{
  sys_msg_t msg;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      if (msg.opcode == CLI_OPCODE_RX) {
        cli_rx_handler();
      }
    }
  }
}

int cli_register_command(const char *name, cli_cmd_fn_t handler)
{
  if (name == NULL || name[0] == '\0' || handler == NULL) {
    return CLI_ERR_PARAM;
  }

  for (size_t i = 0U; i < s_cmd_count; ++i) {
    if (strcmp(s_cmds[i].name, name) == 0) {
      s_cmds[i].fn = handler;
      return CLI_OK;
    }
  }

  if (s_cmd_count >= CLI_CMD_TABLE_MAX) {
    return CLI_ERR_FULL;
  }

  s_cmds[s_cmd_count].name = name;
  s_cmds[s_cmd_count].fn = handler;
  ++s_cmd_count;
  return CLI_OK;
}
