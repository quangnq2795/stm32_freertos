#include "driver.h"

#include "stm32f2xx_hal.h"
#include "clk.h"

void driver_init(void)
{
  HAL_Init();
  clk_init();
}
