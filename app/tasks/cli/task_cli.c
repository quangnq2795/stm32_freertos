#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "task.cli.h"
#include "uart.h"
#include "led.h"

/* CLI over UART0 (uart_id_t = 0). */
#ifndef CLI_UART_ID
#define CLI_UART_ID 0U
#endif

#define CLI_TASK_STACK_WORDS 256U
#define CLI_TASK_PRIO (tskIDLE_PRIORITY + 1U)

static TaskHandle_t s_cliTask = NULL;

/* Blink state (controlled by CLI commands). */
static uint8_t s_blink_enabled = 0U;
static uint8_t s_blink_led_id = 0U;
static uint32_t s_blink_toggle_ms = 500U;
static uint8_t s_blink_dirty = 0U;

/* Simple line editor: accumulate ASCII until '\n' or '\r'. */
static void cli_process_line(char *line, size_t line_len);

static void cli_print(const char *s)
{
  if (s == NULL) {
    return;
  }
  (void)uart_write(CLI_UART_ID, (const uint8_t *)s, strlen(s));
}

typedef void (*cli_cmd_handler_t)(int argc, char **argv);

static void cmd_help(int argc, char **argv);
static void cmd_led(int argc, char **argv);

typedef struct
{
  const char *name;
  cli_cmd_handler_t handler;
} cli_cmd_t;

static const cli_cmd_t s_cmds[] = {
    {"help", cmd_help},
    {"led", cmd_led},
    {0, 0},
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

  /* Init LEDs so app can control them immediately. */
  for (uint8_t i = 0U; i < BSP_LED_COUNT; ++i) {
    led_init(i);
  }

  /* Init UART + register callback (wakeup on RX). */
  uart_init(CLI_UART_ID);
  uart_register_event_callback(CLI_UART_ID, cli_uart_evt_cb);

  /* Next toggle tick (derived from blink state). */
  TickType_t next_toggle_tick = 0;

  char line[64];
  size_t line_len = 0U;

  for (;;) {
    TickType_t now = xTaskGetTickCount();
    TickType_t waitTicks = portMAX_DELAY;

    if (s_blink_enabled != 0U) {
      if (next_toggle_tick <= now) {
        waitTicks = 0;
      } else {
        waitTicks = next_toggle_tick - now;
      }
    }

    /* Wait for UART RX notify or blink timeout */
    (void)ulTaskNotifyTake(pdTRUE, waitTicks);

    /* Drain RX data */
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
        } else {
          if (line_len < (sizeof(line) - 1U)) {
            line[line_len++] = (char)c;
          } else {
            /* Overflow: reset current line. */
            line_len = 0U;
          }
        }
      }
    }

    /* Blink tick */
    if (s_blink_dirty != 0U) {
      s_blink_dirty = 0U;
      now = xTaskGetTickCount();
      next_toggle_tick = now + pdMS_TO_TICKS(s_blink_toggle_ms);
    }

    if (s_blink_enabled != 0U) {
      now = xTaskGetTickCount();
      if (next_toggle_tick <= now) {
        led_toggle((led_id_t)s_blink_led_id);
        next_toggle_tick = now + pdMS_TO_TICKS(s_blink_toggle_ms);
      }
    }
  }
}

static int parse_u32(const char *s, uint32_t *out)
{
  if (s == NULL || *s == '\0') {
    return -1;
  }
  uint32_t v = 0U;
  const char *p = s;
  while (*p != '\0') {
    if (*p < '0' || *p > '9') {
      return -1;
    }
    v = v * 10U + (uint32_t)(*p - '0');
    ++p;
  }
  *out = v;
  return 0;
}

static void cmd_help(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  cli_print("Commands:\r\n");
  cli_print("  led on <id>\r\n");
  cli_print("  led off <id>\r\n");
  cli_print("  led blink <id> [ms]\r\n");
  cli_print("  help\r\n");
}

static void cmd_led(int argc, char **argv)
{
  /* led on <id>
   * led off <id>
   * led blink <id> [ms]
   */
  if (argc < 3) {
    return;
  }
  const char *sub = argv[1];
  const char *id_str = argv[2];
  uint32_t id = 0U;
  if (parse_u32(id_str, &id) != 0U || id >= BSP_LED_COUNT) {
    return;
  }

  if (strcmp(sub, "on") == 0) {
    led_on((led_id_t)id);
    s_blink_enabled = 0U;
    s_blink_dirty = 0U;
    return;
  }

  if (strcmp(sub, "off") == 0) {
    led_off((led_id_t)id);
    s_blink_enabled = 0U;
    s_blink_dirty = 0U;
    return;
  }

  if (strcmp(sub, "blink") == 0) {
    uint32_t ms = 500U;
    if (argc >= 4) {
      const char *ms_str = argv[3];
      uint32_t tmp = 0U;
      if (parse_u32(ms_str, &tmp) == 0U && tmp > 0U) {
        ms = tmp;
      }
    }

    s_blink_led_id = (uint8_t)id;
    s_blink_toggle_ms = ms;
    s_blink_enabled = 1U;
    s_blink_dirty = 1U;
    return;
  }
}

static void cli_process_line(char *line, size_t line_len)
{
  (void)line_len;

  /* Tokenize by spaces/tabs into argv[] */
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

  for (int i = 0; s_cmds[i].name != 0; ++i) {
    if (strcmp(argv[0], s_cmds[i].name) == 0) {
      s_cmds[i].handler(argc, argv);
      return;
    }
  }
}

void task_cli_create(void)
{
  (void)xTaskCreate(task_cli, "cli", CLI_TASK_STACK_WORDS, NULL, CLI_TASK_PRIO, NULL);
}

