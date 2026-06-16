#include "stm32_hal.h"

#include "FreeRTOS.h"
#include "task.h"

/*
 * HAL time base = FreeRTOS tick. Valid only after the scheduler starts and from
 * task context: before vTaskStartScheduler() HAL_GetTick() returns 0 (HAL
 * timeouts do not expire) and HAL_Delay() must not be called.
 */

uint32_t HAL_GetTick(void)
{
  return (uint32_t)xTaskGetTickCount() * (uint32_t)portTICK_PERIOD_MS;
}

void HAL_Delay(uint32_t delay_ms)
{
  if (delay_ms == 0U) {
    return;
  }

  /* Round up to whole ticks so the wait is never shorter than requested
   * (delay_ms >= 1 makes this at least 1 tick). */
  vTaskDelay((TickType_t)((delay_ms + portTICK_PERIOD_MS - 1U) /
                          portTICK_PERIOD_MS));
}
