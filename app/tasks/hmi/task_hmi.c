#include "task_hmi.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "hmi_lcd.h"
#include "hmi_led.h"

#ifndef HMI_CMD_QUEUE_DEPTH
#define HMI_CMD_QUEUE_DEPTH 8U
#endif

#ifndef HMI_TASK_STACK_WORDS
#define HMI_TASK_STACK_WORDS 384U
#endif

#ifndef HMI_TASK_PRIO
#define HMI_TASK_PRIO (tskIDLE_PRIORITY + 1U)
#endif

static QueueHandle_t s_hmi_q;

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

static void task_hmi(void *arg)
{
  (void)arg;

  hmi_led_init();
  hmi_lcd_init();

  for (;;) {
    hmi_msg_t msg;
    if (xQueueReceive(s_hmi_q, &msg, portMAX_DELAY) == pdTRUE) {
      hmi_dispatch(&msg);
    }
  }
}

bool hmi_cmd_send(const hmi_msg_t *msg, TickType_t ticks_to_wait)
{
  if (msg == NULL || s_hmi_q == NULL) {
    return false;
  }
  return xQueueSend(s_hmi_q, msg, ticks_to_wait) == pdTRUE;
}

void task_hmi_create(void)
{
  static uint8_t s_task_started;

  if (s_hmi_q == NULL) {
    s_hmi_q = xQueueCreate(HMI_CMD_QUEUE_DEPTH, sizeof(hmi_msg_t));
  }
  if (s_hmi_q == NULL) {
    return;
  }
  if (s_task_started != 0U) {
    return;
  }
  s_task_started = 1U;
  (void)xTaskCreate(task_hmi, "hmi", HMI_TASK_STACK_WORDS, NULL, HMI_TASK_PRIO, NULL);
}
