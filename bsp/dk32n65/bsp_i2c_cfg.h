#pragma once

#include "stm32_hal.h"

#define BSP_I2C_COUNT 1U

/* Arduino connector I2C1 on STM32N6570-DK: SCL PH9, SDA PC1. */

#define BSP_I2C1_INSTANCE I2C1
/* HAL I2C timing register value for 100 kHz (STM32N6570-DK FSBL template). */
#define BSP_I2C1_CLOCK_SPEED 0x30C0EDFFU
#define BSP_I2C1_SCL_GPIO_PORT GPIOH
#define BSP_I2C1_SCL_GPIO_PIN GPIO_PIN_9
#define BSP_I2C1_SCL_AF GPIO_AF4_I2C1
#define BSP_I2C1_SDA_GPIO_PORT GPIOC
#define BSP_I2C1_SDA_GPIO_PIN GPIO_PIN_1
#define BSP_I2C1_SDA_AF GPIO_AF4_I2C1
#define BSP_I2C1_EV_IRQn I2C1_EV_IRQn
#define BSP_I2C1_ER_IRQn I2C1_ER_IRQn
#define BSP_I2C_IRQ_PRIO 6U

#ifndef BSP_I2C_TIMEOUT_MS
#define BSP_I2C_TIMEOUT_MS 100U
#endif

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
      .ev_irqn = BSP_I2C1_EV_IRQn,                                        \
      .er_irqn = BSP_I2C1_ER_IRQn,                                        \
      .timeout_ms = BSP_I2C_TIMEOUT_MS,                                   \
    },                                                                      \
  }
