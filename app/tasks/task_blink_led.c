#include "FreeRTOS.h"
#include "task.h"
#include "led.h"

static void task_blink_led(void *argument)
{
  (void)argument;
  led_init();

  for (;;) {
    led_toggle();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void task_blink_led_create(void)
{
  (void)xTaskCreate(task_blink_led, "blink", 256, 0, tskIDLE_PRIORITY + 1, 0);
}
