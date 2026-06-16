#include "clk.h"

#include "bsp_clk_cfg.h"

static void clk_halt(void)
{
  for (;;) {
  }
}

/* Bật nguồn (HSE hoặc HSI) và cấu hình PLL: VCO = f_src / PLLM × PLLN. */
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
}

/* Lấy SYSCLK từ PLL, áp flash latency và hệ số chia cho các bus. */
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

void clk_init(void)
{
  clk_configure_osc_pll();
  clk_apply_bus_from_pll();
}

void clk_enable_gpio_port(GPIO_TypeDef *port)
{
  if (port == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (port == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  } else if (port == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  } else if (port == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  } else if (port == GPIOE) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
}

void clk_enable_uart(USART_TypeDef *instance)
{
  if (instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
  } else if (instance == USART3) {
    __HAL_RCC_USART3_CLK_ENABLE();
  }
}

void clk_disable_uart(USART_TypeDef *instance)
{
  if (instance == USART1) {
    __HAL_RCC_USART1_CLK_DISABLE();
  } else if (instance == USART3) {
    __HAL_RCC_USART3_CLK_DISABLE();
  }
}

int clk_config_uart_source(USART_TypeDef *instance)
{
  (void)instance;
  return 0;
}

void clk_enable_i2c(I2C_TypeDef *instance)
{
  if (instance == I2C1) {
    __HAL_RCC_I2C1_CLK_ENABLE();
  }
}

void clk_disable_i2c(I2C_TypeDef *instance)
{
  if (instance == I2C1) {
    __HAL_RCC_I2C1_CLK_DISABLE();
  }
}

int clk_config_i2c_source(I2C_TypeDef *instance)
{
  (void)instance;
  return 0;
}

void clk_enable_tim(TIM_TypeDef *instance)
{
  if (instance == TIM1) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  } else if (instance == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  } else if (instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  } else if (instance == TIM5) {
    __HAL_RCC_TIM5_CLK_ENABLE();
  }
}

void clk_disable_tim(TIM_TypeDef *instance)
{
  if (instance == TIM1) {
    __HAL_RCC_TIM1_CLK_DISABLE();
  } else if (instance == TIM2) {
    __HAL_RCC_TIM2_CLK_DISABLE();
  } else if (instance == TIM3) {
    __HAL_RCC_TIM3_CLK_DISABLE();
  } else if (instance == TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
  } else if (instance == TIM5) {
    __HAL_RCC_TIM5_CLK_DISABLE();
  }
}
