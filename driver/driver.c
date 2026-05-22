#include "driver.h"

#include "clk.h"

void driver_init(void)
{
  HAL_Init();
  clk_init();
}
