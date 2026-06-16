#include "clk.h"

#include "bsp_clk_cfg.h"
#include "stm32_hal.h"

static void clk_halt(void)
{
  for (;;) {
  }
}

/* Cấp nguồn lõi từ nguồn ngoài và đặt voltage scaling cao nhất để chạy 800 MHz. */
static void clk_configure_power(void)
{
  if (HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY) != HAL_OK) {
    clk_halt();
  }

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK) {
    clk_halt();
  }
}

/*
 * Bật oscillator nguồn, chưa chạm tới PLL nào.
 * HSI (64 MHz) luôn bật vì là clock an toàn để rút CPU/SYS về khi cấu hình PLL;
 * bật thêm HSE khi BSP chọn HSE làm nguồn PLL1.
 */
static void clk_enable_oscillators(void)
{
  RCC_OscInitTypeDef osc = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSIDiv = RCC_HSI_DIV1;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
  osc.OscillatorType |= RCC_OSCILLATORTYPE_HSE;
  osc.HSEState = BSP_CLOCK_HSE_STATE;
#endif
  osc.PLL1.PLLState = RCC_PLL_NONE;
  osc.PLL2.PLLState = RCC_PLL_NONE;
  osc.PLL3.PLLState = RCC_PLL_NONE;
  osc.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    clk_halt();
  }
}

/* Nếu CPU/SYS đang chạy từ PLL (qua IC), rút về HSI trước khi cấu hình lại PLL1. */
static void clk_switch_to_hsi(void)
{
  RCC_ClkInitTypeDef clk = {0};

  HAL_RCC_GetClockConfig(&clk);
  if ((clk.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
      (clk.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11)) {
    clk.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK;
    clk.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&clk) != HAL_OK) {
      clk_halt();
    }
  }
}

/* Cấu hình PLL1 từ nguồn BSP chọn: VCO = f_src / PLLM × PLLN, ra = VCO / P1 / P2. */
static void clk_configure_pll1(void)
{
  RCC_OscInitTypeDef osc = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  osc.PLL1.PLLState = RCC_PLL_ON;
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
  osc.PLL1.PLLSource = RCC_PLLSOURCE_HSE;
#else
  osc.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
#endif
  osc.PLL1.PLLM = BSP_CLOCK_PLLM;
  osc.PLL1.PLLN = BSP_CLOCK_PLLN;
  osc.PLL1.PLLFractional = 0;
  osc.PLL1.PLLP1 = BSP_CLOCK_PLLP1;
  osc.PLL1.PLLP2 = BSP_CLOCK_PLLP2;
  osc.PLL2.PLLState = RCC_PLL_NONE;
  osc.PLL3.PLLState = RCC_PLL_NONE;
  osc.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    clk_halt();
  }
}

/* Lấy clock từ PLL1 qua các bộ IC rồi áp hệ số chia cho CPU/SYS và các bus. */
static void clk_apply_bus_from_pll(void)
{
  RCC_ClkInitTypeDef clk = {0};

  clk.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5
                  | RCC_CLOCKTYPE_PCLK4;
  clk.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
  clk.AHBCLKDivider = BSP_CLOCK_AHB_DIV;
  clk.APB1CLKDivider = BSP_CLOCK_APB1_DIV;
  clk.APB2CLKDivider = BSP_CLOCK_APB2_DIV;
  clk.APB4CLKDivider = BSP_CLOCK_APB4_DIV;
  clk.APB5CLKDivider = BSP_CLOCK_APB5_DIV;
  clk.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk.IC1Selection.ClockDivider = BSP_CLOCK_IC1_DIV;
  clk.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk.IC2Selection.ClockDivider = BSP_CLOCK_IC2_DIV;
  clk.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk.IC6Selection.ClockDivider = BSP_CLOCK_IC6_DIV;
  clk.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  clk.IC11Selection.ClockDivider = BSP_CLOCK_IC11_DIV;

  if (HAL_RCC_ClockConfig(&clk) != HAL_OK) {
    clk_halt();
  }
}

void clk_init(void)
{
  clk_configure_power();
  clk_enable_oscillators();
  clk_switch_to_hsi();
  clk_configure_pll1();
  clk_apply_bus_from_pll();

  SystemCoreClockUpdate();
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
  } else if (port == GPIOF) {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  } else if (port == GPIOG) {
    __HAL_RCC_GPIOG_CLK_ENABLE();
  } else if (port == GPIOH) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
  } else if (port == GPIOO) {
    __HAL_RCC_GPIOO_CLK_ENABLE();
  }
}

void clk_enable_uart(USART_TypeDef *instance)
{
  if (instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
  }
}

void clk_disable_uart(USART_TypeDef *instance)
{
  if (instance == USART1) {
    __HAL_RCC_USART1_CLK_DISABLE();
  }
}

int clk_config_uart_source(USART_TypeDef *instance)
{
  RCC_PeriphCLKInitTypeDef pc = {0};

  if (instance == USART1) {
    pc.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    pc.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    return (HAL_RCCEx_PeriphCLKConfig(&pc) == HAL_OK) ? 0 : -1;
  }

  return -1;
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
  RCC_PeriphCLKInitTypeDef pc = {0};

  if (instance == I2C1) {
    pc.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    pc.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    return (HAL_RCCEx_PeriphCLKConfig(&pc) == HAL_OK) ? 0 : -1;
  }

  return -1;
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
  }
}
