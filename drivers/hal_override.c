#include "stm32f2xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t HAL_GetTick(void)
{
  return (uint32_t)xTaskGetTickCount() * (uint32_t)portTICK_PERIOD_MS;
}

void HAL_Delay(uint32_t Delay)
{
  TickType_t ticks;

  if (Delay == 0U)
  {
    return;
  }

  ticks = (TickType_t)(Delay / portTICK_PERIOD_MS);
  if ((Delay % portTICK_PERIOD_MS) != 0U)
  {
    ticks++;
  }

  if (ticks == 0U)
  {
    ticks = 1;
  }

  vTaskDelay(ticks);
}
