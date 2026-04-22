#include "cli_cmd_led.h"

#include "led.h"
#include "hmi_msg.h"
#include "task.cli.h"
#include "task_hmi.h"

#include <stdlib.h>
#include <string.h>

static bool led_parse_index(const char *s, uint8_t *out_id)
{
  if (s == NULL || out_id == NULL) {
    return false;
  }
  char *end = NULL;
  unsigned long v = strtoul(s, &end, 0);
  if (end == s || *end != '\0') {
    return false;
  }
  /* CLI: led 1 = BSP index 0 */
  if (v < 1UL || v > (unsigned long)LED_COUNT) {
    return false;
  }
  *out_id = (uint8_t)(v - 1UL);
  return true;
}

static void cmd_led(int argc, char **argv)
{
  if (argc < 3) {
    cli_print("usage: led <1..N> on|off|blink <period_ms>\r\n");
    return;
  }

  uint8_t led_id;
  if (!led_parse_index(argv[1], &led_id)) {
    cli_print("bad led index\r\n");
    return;
  }

  hmi_msg_t msg;
  memset(&msg, 0, sizeof(msg));

  if (strcmp(argv[2], "on") == 0) {
    msg.type = HMI_MSG_LED_SET;
    msg.u.led_set.led_id = led_id;
    msg.u.led_set.on = 1U;
  } else if (strcmp(argv[2], "off") == 0) {
    msg.type = HMI_MSG_LED_SET;
    msg.u.led_set.led_id = led_id;
    msg.u.led_set.on = 0U;
  } else if (strcmp(argv[2], "blink") == 0) {
    if (argc < 4) {
      cli_print("usage: led <n> blink <period_ms>\r\n");
      return;
    }
    char *end = NULL;
    unsigned long per = strtoul(argv[3], &end, 0);
    if (end == argv[3] || *end != '\0' || per > 0xFFFFUL) {
      cli_print("bad period\r\n");
      return;
    }
    msg.type = HMI_MSG_LED_BLINK;
    msg.u.led_blink.led_id = led_id;
    msg.u.led_blink.period_ms = (uint16_t)per;
  } else {
    cli_print("unknown led subcommand\r\n");
    return;
  }

  if (!hmi_cmd_send(&msg, pdMS_TO_TICKS(50))) {
    cli_print("hmi queue full\r\n");
  }
}

const cli_cmd_entry_t cli_led_cmds[] = {
    {"led", cmd_led},
    {0, 0},
};
