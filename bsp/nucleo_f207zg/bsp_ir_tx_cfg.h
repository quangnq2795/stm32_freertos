#pragma once

#include "stm32f2xx_hal.h"

/* KY-005 S -> PA8 TIM1_CH1 (38 kHz PWM). Adjust if wiring differs. */

#define BSP_IR_TX_COUNT  1U

#define BSP_IR_TX1_GPIO_PORT         GPIOA
#define BSP_IR_TX1_GPIO_PIN          GPIO_PIN_8
#define BSP_IR_TX1_GPIO_AF           GPIO_AF1_TIM1
#define BSP_IR_TX1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define BSP_IR_TX1_TIM_PWM              TIM1
#define BSP_IR_TX1_TIM_PWM_CLK_ENABLE() __HAL_RCC_TIM1_CLK_ENABLE()
#define BSP_IR_TX1_TIM_PWM_CH           TIM_CHANNEL_1
#define BSP_IR_TX1_TIM_PWM_PRESCALER    0U
#define BSP_IR_TX1_TIM_PWM_PERIOD       3157U
#define BSP_IR_TX1_TIM_PWM_PULSE        1052U

#define DEFINE_IR_TX_CLK_ENABLE_FUNC(name, macro) \
  static inline void name(void) { macro(); }

DEFINE_IR_TX_CLK_ENABLE_FUNC(ir_tx1_gpio_clk_enable, BSP_IR_TX1_GPIO_CLK_ENABLE)
DEFINE_IR_TX_CLK_ENABLE_FUNC(ir_tx1_tim_clk_enable, BSP_IR_TX1_TIM_PWM_CLK_ENABLE)

#define BSP_IR_TX_DESCS                                                     \
  {                                                                         \
    [0] = {                                                                 \
      .hw = {                                                               \
        .gpio_port = BSP_IR_TX1_GPIO_PORT,                                 \
        .gpio_pin = BSP_IR_TX1_GPIO_PIN,                                   \
        .gpio_af = BSP_IR_TX1_GPIO_AF,                                     \
        .tim_pwm = BSP_IR_TX1_TIM_PWM,                                     \
        .tim_channel = BSP_IR_TX1_TIM_PWM_CH,                              \
        .tim_prescaler = BSP_IR_TX1_TIM_PWM_PRESCALER,                     \
        .tim_period = BSP_IR_TX1_TIM_PWM_PERIOD,                           \
        .tim_pulse = BSP_IR_TX1_TIM_PWM_PULSE,                             \
        .gpio_clk_enable = ir_tx1_gpio_clk_enable,                         \
        .tim_clk_enable = ir_tx1_tim_clk_enable,                           \
      },                                                                    \
    },                                                                      \
  }
