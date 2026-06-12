#pragma once

#include "stm32f2xx_hal.h"

#define BSP_I2C_COUNT 1U

/* BSP_I2C1 = Arduino D14/D15 (I2C1 on PB9/PB8). */

#define BSP_I2C1_INSTANCE I2C1
#define BSP_I2C1_CLOCK_SPEED 100000U
#define BSP_I2C1_SCL_GPIO_PORT GPIOB
#define BSP_I2C1_SCL_GPIO_PIN GPIO_PIN_8
#define BSP_I2C1_SCL_AF GPIO_AF4_I2C1
#define BSP_I2C1_SDA_GPIO_PORT GPIOB
#define BSP_I2C1_SDA_GPIO_PIN GPIO_PIN_9
#define BSP_I2C1_SDA_AF GPIO_AF4_I2C1
#define BSP_I2C1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define BSP_I2C1_CLK_ENABLE() __HAL_RCC_I2C1_CLK_ENABLE()
#define BSP_I2C1_EV_IRQn I2C1_EV_IRQn
#define BSP_I2C1_ER_IRQn I2C1_ER_IRQn
/* Same as UART / IR driver IRQ priority (no FreeRTOS API in ISR). */
#define BSP_I2C_IRQ_PRIO 6U

#ifndef BSP_I2C_TIMEOUT_MS
#define BSP_I2C_TIMEOUT_MS 100U
#endif

#define DEFINE_I2C_CLK_ENABLE_FUNC(name, macro) \
  static inline void name(void) { macro(); }

DEFINE_I2C_CLK_ENABLE_FUNC(i2c1_gpio_clk_enable, BSP_I2C1_GPIO_CLK_ENABLE)
DEFINE_I2C_CLK_ENABLE_FUNC(i2c1_i2c_clk_enable, BSP_I2C1_CLK_ENABLE)

#ifndef BSP_I2C_EV_IRQ_HANDLER
#define BSP_I2C_EV_IRQ_HANDLER(I2Cn) \
  void I2Cn##_EV_IRQHandler(void) { i2c_ev_irq_handler(I2Cn); }
#endif

#ifndef BSP_I2C_ER_IRQ_HANDLER
#define BSP_I2C_ER_IRQ_HANDLER(I2Cn) \
  void I2Cn##_ER_IRQHandler(void) { i2c_er_irq_handler(I2Cn); }
#endif

#define BSP_I2C_IRQ_HANDLERS \
  BSP_I2C_EV_IRQ_HANDLER(I2C1) \
  BSP_I2C_ER_IRQ_HANDLER(I2C1)

#define BSP_I2C_DESCS                                                       \
  {                                                                          \
    [0] = {                                                                 \
      .instance = BSP_I2C1_INSTANCE,                                      \
      .clock_speed = BSP_I2C1_CLOCK_SPEED,                                \
      .scl_port = BSP_I2C1_SCL_GPIO_PORT,                                 \
      .scl_pin = BSP_I2C1_SCL_GPIO_PIN,                                   \
      .scl_af = BSP_I2C1_SCL_AF,                                            \
      .sda_port = BSP_I2C1_SDA_GPIO_PORT,                                 \
      .sda_pin = BSP_I2C1_SDA_GPIO_PIN,                                   \
      .sda_af = BSP_I2C1_SDA_AF,                                            \
      .gpio_clk_enable = i2c1_gpio_clk_enable,                            \
      .i2c_clk_enable = i2c1_i2c_clk_enable,                              \
      .ev_irqn = BSP_I2C1_EV_IRQn,                                        \
      .er_irqn = BSP_I2C1_ER_IRQn,                                        \
      .timeout_ms = BSP_I2C_TIMEOUT_MS,                                   \
    },                                                                      \
  }
