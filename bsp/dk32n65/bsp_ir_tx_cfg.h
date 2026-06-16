#pragma once

#include "stm32_hal.h"

/* Placeholder IR TX pin — customize for your DK32N65 wiring. */

#define BSP_IR_TX_COUNT  1U

#define BSP_IR_TX1_GPIO_PORT         GPIOA
#define BSP_IR_TX1_GPIO_PIN          GPIO_PIN_1
#define BSP_IR_TX1_GPIO_AF           GPIO_AF2_TIM3

#define BSP_IR_TX1_TIM_PWM              TIM3
#define BSP_IR_TX1_TIM_PWM_CH           TIM_CHANNEL_1
#define BSP_IR_TX1_TIM_PWM_PRESCALER    0U
#define BSP_IR_TX1_TIM_PWM_PERIOD       3157U
#define BSP_IR_TX1_TIM_PWM_PULSE        1052U

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
      },                                                                    \
    },                                                                      \
  }
