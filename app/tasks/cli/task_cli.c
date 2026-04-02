#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "cli_cmd.h"
#include "cli_cmd_led.h"
#include "cli_cmd_sensor.h"
#include "task.cli.h"
#include "uart.h"

#ifndef CLI_UART_ID
#define CLI_UART_ID 0U
#endif

#define CLI_TASK_STACK_WORDS 512U
#define CLI_TASK_PRIO (tskIDLE_PRIORITY + 1U)

static TaskHandle_t s_cliTask = NULL;

static void cli_process_line(char *line, size_t line_len);

static void cli_print(const char *s)
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
    cli_led_cmds,
    cli_sensor_cmds,
};

static void cli_uart_evt_cb(uart_id_t id, uart_event_t evt)
{
  (void)id;
  if (evt != UART_EVENT_RX) {
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  if (s_cliTask != NULL) {
    vTaskNotifyGiveFromISR(s_cliTask, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

static void task_cli(void *argument)
{
  (void)argument;

  s_cliTask = xTaskGetCurrentTaskHandle();

  uart_init(CLI_UART_ID);
  uart_register_event_callback(CLI_UART_ID, cli_uart_evt_cb);

  char line[64];
  size_t line_len = 0U;

  for (;;) {
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    while (uart_rx_available(CLI_UART_ID) > 0U) {
      uint8_t buf[32];
      size_t n = uart_read(CLI_UART_ID, buf, sizeof(buf));
      (void)uart_write(CLI_UART_ID, buf, n);
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
}

static void cmd_help(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  cli_print("Commands:\r\n");
  cli_print("  help\r\n");
  cli_print("  led ...\r\n");
  cli_print("  sensor ...\r\n");
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
  (void)xTaskCreate(task_cli, "cli", CLI_TASK_STACK_WORDS, NULL, CLI_TASK_PRIO, NULL);
}
