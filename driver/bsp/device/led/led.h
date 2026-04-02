#pragma once

#include <stdint.h>
#include "stm32f2xx_hal.h"
#include "bsp_led_cfg.h"
/* LED identifier (numeric ID: 0..BSP_LED_COUNT-1) */
typedef uint8_t led_id_t;

typedef enum
{
  led_state_off = 0,
  led_state_on = 1,
} led_state_t;

typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  led_state_t state;
  uint32_t blink_cycles;
  void (*clk_enable)(void);
} led_desc_t;
 
void led_init(led_id_t id);
void led_on(led_id_t id);
void led_off(led_id_t id);
void led_toggle(led_id_t id);

