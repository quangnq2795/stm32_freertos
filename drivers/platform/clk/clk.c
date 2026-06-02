#include "clk.h"

#include "bsp_clk_cfg.h"

static void clk_halt(void)
{
  for (;;) {
  }
}

static void clk_apply_bus_from_pll(void)
{
  RCC_ClkInitTypeDef clk = {0};

  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = BSP_CLOCK_AHB_DIV;
  clk.APB1CLKDivider = BSP_CLOCK_APB1_DIV;
  clk.APB2CLKDivider = BSP_CLOCK_APB2_DIV;

  if (HAL_RCC_ClockConfig(&clk, BSP_CLOCK_FLASH_LATENCY) != HAL_OK) {
    clk_halt();
  }
}

static void clk_configure_osc_pll(void)
{
  RCC_OscInitTypeDef osc = {0};

#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSI
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  osc.PLL.PLLM = BSP_CLOCK_PLLM_HSI;
#else
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = BSP_CLOCK_HSE_STATE;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLM = BSP_CLOCK_PLLM_HSE;
#endif

  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLN = BSP_CLOCK_PLLN;
  osc.PLL.PLLP = BSP_CLOCK_PLLP;
  osc.PLL.PLLQ = BSP_CLOCK_PLLQ;

  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    clk_halt();
  }

  clk_apply_bus_from_pll();
}

void clk_init(void)
{
  clk_configure_osc_pll();
}
