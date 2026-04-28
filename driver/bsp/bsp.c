#include "bsp.h"
#include "clk.h"
#include "stm32f2xx_hal.h"


void bsp_init(void)
{
  HAL_Init();
  clk_init();
}

void bsp_sync_systick(void)
{
  HAL_IncTick();
}
