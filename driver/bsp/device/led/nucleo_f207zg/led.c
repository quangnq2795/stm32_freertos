#include "led.h"
#include "bsp_cfg.h"

void led_init(void)
{
  BSP_LED_GPIO_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = BSP_LED_GPIO_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BSP_LED_GPIO_PORT, &gpio);
}

void led_on(void)
{
  HAL_GPIO_WritePin(BSP_LED_GPIO_PORT, BSP_LED_GPIO_PIN, GPIO_PIN_SET);
}

void led_off(void)
{
  HAL_GPIO_WritePin(BSP_LED_GPIO_PORT, BSP_LED_GPIO_PIN, GPIO_PIN_RESET);
}

void led_toggle(void)
{
  HAL_GPIO_TogglePin(BSP_LED_GPIO_PORT, BSP_LED_GPIO_PIN);
}
