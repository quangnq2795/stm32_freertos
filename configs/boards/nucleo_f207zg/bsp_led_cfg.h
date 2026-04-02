#pragma once

#define BSP_LED_COUNT 3U

/* LED mapping (led1/2/3) -> physical pins */
#define BSP_LED1_GPIO_PORT GPIOB
#define BSP_LED1_GPIO_PIN  GPIO_PIN_0
#define BSP_LED1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define BSP_LED1_BLINK_CYCLES 0U

#define BSP_LED2_GPIO_PORT GPIOB
#define BSP_LED2_GPIO_PIN  GPIO_PIN_7
#define BSP_LED2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define BSP_LED2_BLINK_CYCLES 0U

#define BSP_LED3_GPIO_PORT GPIOB
#define BSP_LED3_GPIO_PIN  GPIO_PIN_14
#define BSP_LED3_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define BSP_LED3_BLINK_CYCLES 0U

/* Helper macro: generate a small wrapper around LED clock-enable macros.
 * We generate functions because `led_desc_t` stores a function pointer.
 *
 * Pass macro name without trailing `()` (e.g. BSP_LED2_GPIO_CLK_ENABLE).
 */
#define DEFINE_LED_CLK_ENABLE_FUNC(name, macro) \
  static inline void name(void) { macro(); }

DEFINE_LED_CLK_ENABLE_FUNC(led1_clk_enable, BSP_LED1_GPIO_CLK_ENABLE)
DEFINE_LED_CLK_ENABLE_FUNC(led2_clk_enable, BSP_LED2_GPIO_CLK_ENABLE)
DEFINE_LED_CLK_ENABLE_FUNC(led3_clk_enable, BSP_LED3_GPIO_CLK_ENABLE)

/* Initializer for generic LED driver.
 * Uses the wrapper function symbols generated above.
 */
#define BSP_LED_DESCS                                                       \
  {                                                                          \
    [0] = {BSP_LED1_GPIO_PORT, BSP_LED1_GPIO_PIN, led_state_off,         \
           BSP_LED1_BLINK_CYCLES, led1_clk_enable},                         \
    [1] = {BSP_LED2_GPIO_PORT, BSP_LED2_GPIO_PIN, led_state_off,         \
           BSP_LED2_BLINK_CYCLES, led2_clk_enable},                         \
    [2] = {BSP_LED3_GPIO_PORT, BSP_LED3_GPIO_PIN, led_state_off,         \
           BSP_LED3_BLINK_CYCLES, led3_clk_enable},                         \
  }

