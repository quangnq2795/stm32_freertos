#include "driver.h"

#include "stm32_hal.h"
#include "clk.h"

void driver_init(void)
{
  HAL_Init();
  clk_init();
}
