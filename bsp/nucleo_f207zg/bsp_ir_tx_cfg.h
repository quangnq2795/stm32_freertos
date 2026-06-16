#pragma once

#include "stm32_hal.h"

/* KY-005 S -> PA8 TIM1_CH1 (38 kHz PWM). Adjust if wiring differs. */

#define BSP_IR_TX_COUNT  1U

#define BSP_IR_TX1_GPIO_PORT         GPIOA
#define BSP_IR_TX1_GPIO_PIN          GPIO_PIN_8
#define BSP_IR_TX1_GPIO_AF           GPIO_AF1_TIM1

#define BSP_IR_TX1_TIM_PWM              TIM1
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
