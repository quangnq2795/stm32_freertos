#include "led.h"
#include "bsp_led_cfg.h"

#include "gpio.h"

static led_desc_t g_leds[LED_COUNT] = BSP_LED_DESCS;

void led_init(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  gpio_config_output(g_leds[id].port, g_leds[id].pin, GPIO_OUT_PUSH_PULL,
                     GPIO_NOPULL,
                     (g_leds[id].state == led_state_on) ? GPIO_LEVEL_HIGH
                                                        : GPIO_LEVEL_LOW);
}

void led_on(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  g_leds[id].state = led_state_on;
  gpio_set(g_leds[id].port, g_leds[id].pin);
}

void led_off(led_id_t id)
{
  if (id >= LED_COUNT) {
    return;
  }

  g_leds[id].state = led_state_off;
  gpio_clear(g_leds[id].port, g_leds[id].pin);
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
