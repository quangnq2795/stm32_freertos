#include "led.h"
#include "bsp_led_cfg.h"

#include "stm32f2xx_hal.h"

static led_desc_t g_leds[LED_COUNT] = BSP_LED_DESCS;

void led_init(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  if (g_leds[id].clk_enable != 0) {
    g_leds[id].clk_enable();
  }

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = g_leds[id].pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(g_leds[id].port, &gpio);

  /* Apply initial state to GPIO (BLINK starts from OFF level). */
  if (g_leds[id].state == led_state_on) {
    HAL_GPIO_WritePin(g_leds[id].port, g_leds[id].pin, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(g_leds[id].port, g_leds[id].pin, GPIO_PIN_RESET);
  }
}

void led_on(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  g_leds[id].state = led_state_on;
  g_leds[id].blink_cycles = 0U;
  HAL_GPIO_WritePin(g_leds[id].port, g_leds[id].pin, GPIO_PIN_SET);
}

void led_off(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  g_leds[id].state = led_state_off;
  g_leds[id].blink_cycles = 0U;
  HAL_GPIO_WritePin(g_leds[id].port, g_leds[id].pin, GPIO_PIN_RESET);
}

void led_toggle(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  if (g_leds[id].state == led_state_on) {
    led_off(id);
  } else if (g_leds[id].state == led_state_off) {
    led_on(id);
  }
}

