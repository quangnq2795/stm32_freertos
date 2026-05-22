#include "task_hmi.h"

#include <string.h>

#include "taskmanager.h"

#include "hmi_lcd.h"
#include "hmi_led.h"

#ifndef HMI_TASK_STACK_WORDS
#define HMI_TASK_STACK_WORDS 384U
#endif

#ifndef HMI_TASK_PRIO
#define HMI_TASK_PRIO (tskIDLE_PRIORITY + 1U)
#endif

static void hmi_dispatch(const hmi_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  switch (msg->type) {
  case HMI_MSG_LED_SET:
  case HMI_MSG_LED_BLINK:
    hmi_led_on_msg(msg);
    break;
  case HMI_MSG_LCD_TEXT:
    hmi_lcd_on_msg(msg);
    break;
  default:
    break;
  }
}

static bool hmi_sys_to_local(const sys_msg_t *in, hmi_msg_t *out)
{
  if (in == NULL || out == NULL) {
    return false;
  }

  memset(out, 0, sizeof(*out));

  switch (in->opcode) {
  case SYS_NODE_HMI_OPCODE_LED_SET:
    out->type = HMI_MSG_LED_SET;
    out->u.led_set.led_id = (uint8_t)in->u.arg.param1;
    out->u.led_set.on = (uint8_t)in->u.arg.param2;
    return true;
  case SYS_NODE_HMI_OPCODE_LED_BLINK:
    out->type = HMI_MSG_LED_BLINK;
    out->u.led_blink.led_id = (uint8_t)in->u.arg.param1;
    out->u.led_blink.period_ms = (uint16_t)in->u.arg.param2;
    return true;
  case SYS_NODE_HMI_OPCODE_LCD_TEXT:
    out->type = HMI_MSG_LCD_TEXT;
    out->u.lcd_text.row = in->u.lcd.row;
    out->u.lcd_text.col = in->u.lcd.col;
    strncpy(out->u.lcd_text.text, in->u.lcd.text, HMI_LCD_TEXT_MAX - 1U);
    out->u.lcd_text.text[HMI_LCD_TEXT_MAX - 1U] = '\0';
    return true;
  default:
    return false;
  }
}

static void hmi_local_to_sys(const hmi_msg_t *in, sys_msg_t *out)
{
  memset(out, 0, sizeof(*out));
  out->dst = (uint32_t)SYS_NODE_HMI;

  switch (in->type) {
  case HMI_MSG_LED_SET:
    out->opcode = SYS_NODE_HMI_OPCODE_LED_SET;
    out->u.arg.param1 = in->u.led_set.led_id;
    out->u.arg.param2 = in->u.led_set.on;
    break;
  case HMI_MSG_LED_BLINK:
    out->opcode = SYS_NODE_HMI_OPCODE_LED_BLINK;
    out->u.arg.param1 = in->u.led_blink.led_id;
    out->u.arg.param2 = in->u.led_blink.period_ms;
    break;
  case HMI_MSG_LCD_TEXT:
    out->opcode = SYS_NODE_HMI_OPCODE_LCD_TEXT;
    out->u.lcd.row = in->u.lcd_text.row;
    out->u.lcd.col = in->u.lcd_text.col;
    strncpy(out->u.lcd.text, in->u.lcd_text.text, SYS_MSG_LCD_TEXT_MAX - 1U);
    out->u.lcd.text[SYS_MSG_LCD_TEXT_MAX - 1U] = '\0';
    break;
  default:
    break;
  }
}

static void hmi_on_msg(const sys_msg_t *msg, void *ctx)
{
  hmi_msg_t local;

  (void)ctx;

  if (!hmi_sys_to_local(msg, &local)) {
    return;
  }
  hmi_dispatch(&local);
}

bool hmi_cmd_send(const hmi_msg_t *msg, TickType_t ticks_to_wait)
{
  sys_msg_t sys_msg;

  if (msg == NULL) {
    return false;
  }

  hmi_local_to_sys(msg, &sys_msg);
  return tm_send(SYS_NODE_HMI, &sys_msg, ticks_to_wait) == TM_OK;
}

void task_hmi_create(void)
{
  static uint8_t s_started;
  static const tm_listen_t s_hmi_listen[] = {
      {.from = SYS_NODE_CLI, .queue_len = 8U},
  };
  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_HMI,
      .name = "hmi",
      .handler = hmi_on_msg,
      .stack_words = HMI_TASK_STACK_WORDS,
      .priority = HMI_TASK_PRIO,
      .listen = s_hmi_listen,
      .listen_count = 1U,
  };

  if (s_started != 0U) {
    return;
  }
  s_started = 1U;

  hmi_led_init();
  hmi_lcd_init();
  (void)tm_init(&cfg);
}
