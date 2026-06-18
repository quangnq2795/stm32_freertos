#pragma once

#include "stm32_hal.h"

/* KY-022 OUT -> PC0 (EXTI0). Adjust if wiring differs. */

#define BSP_IR_RX_COUNT  1U

#define BSP_IR_RX1_GPIO_PORT         GPIOC
#define BSP_IR_RX1_GPIO_PIN          GPIO_PIN_0
#define BSP_IR_RX1_EXTI_IRQn         EXTI0_IRQn
#define BSP_IR_RX1_EXTI_LINE         EXTI_LINE_0
#define BSP_IR_RX1_EXTI_IRQ_PRIO     6U
#define BSP_IR_RX1_IDLE_TIMEOUT_US   20000U
#define BSP_IR_RX1_PULSE_RING_CAP    128U

#define BSP_IR_RX_MAX_PULSE_RING_CAP  BSP_IR_RX1_PULSE_RING_CAP

#define BSP_IR_RX_DESCS                                                     \
  {                                                                         \
    [0] = {                                                                 \
      .hw = {                                                               \
        .gpio_port = BSP_IR_RX1_GPIO_PORT,                                 \
        .gpio_pin = BSP_IR_RX1_GPIO_PIN,                                   \
        .exti_irqn = BSP_IR_RX1_EXTI_IRQn,                                 \
        .exti_irq_prio = BSP_IR_RX1_EXTI_IRQ_PRIO,                         \
        .idle_timeout_us = BSP_IR_RX1_IDLE_TIMEOUT_US,                     \
        .pulse_ring_cap = BSP_IR_RX1_PULSE_RING_CAP,                       \
      },                                                                    \
    },                                                                      \
  }

/* EXTI vector handlers are owned by the gpio platform driver (gpio.c); this BSP
 * only selects the line via BSP_IR_RX1_EXTI_IRQn and registers a callback in
 * ir_rx_drv_init() through gpio_config_input_exti(). */
