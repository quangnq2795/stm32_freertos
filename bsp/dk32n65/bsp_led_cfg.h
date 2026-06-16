#pragma once

#include "stm32_hal.h"

#define BSP_LED_COUNT 2U

#define BSP_LED1_GPIO_PORT GPIOO
#define BSP_LED1_GPIO_PIN  GPIO_PIN_1

#define BSP_LED2_GPIO_PORT GPIOG
#define BSP_LED2_GPIO_PIN  GPIO_PIN_10
#define BSP_LED_DESCS                                                       \
  {                                                                          \
    [0] = {BSP_LED1_GPIO_PORT, BSP_LED1_GPIO_PIN, led_state_off},           \
    [1] = {BSP_LED2_GPIO_PORT, BSP_LED2_GPIO_PIN, led_state_off},           \
  }
