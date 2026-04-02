#include "bsp.h"
#include "bsp_cfg.h"

static void SystemClock_ApplyBusFromPll(void);
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
static void SystemClock_Config_HSE(void);
#endif
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSI
static void SystemClock_Config_HSI(void);
#endif
static void SystemClock_Config(void);


/** SYSCLK từ PLL + chia AHB/APB (chung cho HSE và HSI). */
static void SystemClock_ApplyBusFromPll(void)
{
  RCC_ClkInitTypeDef clk = {0};

  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = BSP_CLOCK_AHB_DIV;
  clk.APB1CLKDivider = BSP_CLOCK_APB1_DIV;
  clk.APB2CLKDivider = BSP_CLOCK_APB2_DIV;
  if (HAL_RCC_ClockConfig(&clk, BSP_CLOCK_FLASH_LATENCY) != HAL_OK) {
    for (;;) {
    }
  }
}

#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
static void SystemClock_Config_HSE(void)
{
  RCC_OscInitTypeDef osc = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = BSP_CLOCK_HSE_STATE;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  osc.PLL.PLLM = BSP_CLOCK_PLLM_HSE;
  osc.PLL.PLLN = BSP_CLOCK_PLLN;
  osc.PLL.PLLP = BSP_CLOCK_PLLP;
  osc.PLL.PLLQ = BSP_CLOCK_PLLQ;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    for (;;) {
    }
  }
  SystemClock_ApplyBusFromPll();
}
#endif

#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSI
static void SystemClock_Config_HSI(void)
{
  RCC_OscInitTypeDef osc = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  osc.PLL.PLLM = BSP_CLOCK_PLLM_HSI;
  osc.PLL.PLLN = BSP_CLOCK_PLLN;
  osc.PLL.PLLP = BSP_CLOCK_PLLP;
  osc.PLL.PLLQ = BSP_CLOCK_PLLQ;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    for (;;) {
    }
  }
  SystemClock_ApplyBusFromPll();
}
#endif

/** Chọn HSE hoặc HSI theo `BSP_CLOCK_PLL_SRC` trong `bsp_cfg.h`. */
static void SystemClock_Config(void)
{
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSI
  SystemClock_Config_HSI();
#else
  SystemClock_Config_HSE();
#endif
}


void bsp_init(void)
{
  HAL_Init();
  SystemClock_Config();
}

void sync_systick(void)
{
  HAL_IncTick();
}
