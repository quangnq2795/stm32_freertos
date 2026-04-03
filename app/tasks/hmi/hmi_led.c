#include "hmi_led.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "bsp_led_cfg.h"
#include "led.h"

#include <stdint.h>

static TimerHandle_t s_led_timer[BSP_LED_COUNT];
static char s_led_timer_name[BSP_LED_COUNT][4];

static void hmi_led_timer_cb(TimerHandle_t t)
{
  uintptr_t const id = (uintptr_t)pvTimerGetTimerID(t);
  if (id < (uintptr_t)BSP_LED_COUNT) {
    led_toggle((led_id_t)id);
  }
}

static void hmi_led_timer_ensure(led_id_t id)
{
  if (id >= BSP_LED_COUNT || s_led_timer[id] != NULL) {
    return;
  }
  s_led_timer[id] = xTimerCreate(s_led_timer_name[id], 1U, pdTRUE, (void *)(uintptr_t)id,
                                 hmi_led_timer_cb);
}

static void hmi_led_timer_stop(led_id_t id)
{
  if (id >= BSP_LED_COUNT || s_led_timer[id] == NULL) {
    return;
  }
  (void)xTimerStop(s_led_timer[id], portMAX_DELAY);
}

void hmi_led_init(void)
{
  for (led_id_t i = 0U; i < BSP_LED_COUNT; ++i) {
    s_led_timer_name[i][0] = 'l';
    s_led_timer_name[i][1] = (char)('0' + (unsigned)i);
    s_led_timer_name[i][2] = '\0';
    s_led_timer[i] = NULL;
    led_init(i);
  }
}

void hmi_led_on_msg(const hmi_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  switch (msg->type) {
  case HMI_MSG_LED_SET: {
    led_id_t const id = msg->u.led_set.led_id;
    if (id >= BSP_LED_COUNT) {
      break;
    }
    hmi_led_timer_stop(id);
    if (msg->u.led_set.on != 0U) {
      led_on(id);
    } else {
      led_off(id);
    }
    break;
  }
  case HMI_MSG_LED_BLINK: {
    led_id_t const id = msg->u.led_blink.led_id;
    if (id >= BSP_LED_COUNT) {
      break;
    }
    if (msg->u.led_blink.period_ms == 0U) {
      hmi_led_timer_stop(id);
      led_off(id);
      break;
    }
    hmi_led_timer_ensure(id);
    if (s_led_timer[id] == NULL) {
      break;
    }
    TickType_t half = pdMS_TO_TICKS((uint32_t)msg->u.led_blink.period_ms / 2U);
    if (half == 0U) {
      half = 1U;
    }
    led_off(id);
    (void)xTimerChangePeriod(s_led_timer[id], half, portMAX_DELAY);
    (void)xTimerStart(s_led_timer[id], portMAX_DELAY);
    break;
  }
  default:
    break;
  }
}
