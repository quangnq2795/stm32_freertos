#pragma once

#include "stm32_hal.h"

#define BSP_LED_COUNT 3U

/* LED mapping (led1/2/3) -> physical pins */
#define BSP_LED1_GPIO_PORT GPIOB
#define BSP_LED1_GPIO_PIN  GPIO_PIN_0

#define BSP_LED2_GPIO_PORT GPIOB
#define BSP_LED2_GPIO_PIN  GPIO_PIN_7

#define BSP_LED3_GPIO_PORT GPIOB
#define BSP_LED3_GPIO_PIN  GPIO_PIN_14
#define BSP_LED_DESCS                                                       \
  {                                                                          \
    [0] = {BSP_LED1_GPIO_PORT, BSP_LED1_GPIO_PIN, led_state_off},           \
    [1] = {BSP_LED2_GPIO_PORT, BSP_LED2_GPIO_PIN, led_state_off},           \
    [2] = {BSP_LED3_GPIO_PORT, BSP_LED3_GPIO_PIN, led_state_off},           \
  }

