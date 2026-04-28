#include "bsp.h"
#include "clk.h"
#include "stm32f2xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

/**
 * Override weak HAL_GetTick: sau khi FreeRTOS chạy, dùng tick RTOS (ms khi
 * configTICK_RATE_HZ == 1000). Trước scheduler vẫn dùng uwTick (HAL_IncTick).
 */
uint32_t HAL_GetTick(void)
{
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    return (uint32_t)xTaskGetTickCount();
  }
  return (uint32_t)uwTick;
}

void bsp_init(void)
{
  HAL_Init();
  clk_init();
}
