#include "gpio.h"

#include "clk.h"

/* ---- Basic GPIO control ---- */

/* Enable the port clock and apply one pin configuration (shared setup path). */
static void gpio_pin_init(GPIO_TypeDef *port, uint16_t pin, uint32_t mode,
                          uint32_t pull)
{
  GPIO_InitTypeDef gpio = {0};

  clk_enable_gpio_port(port);

  gpio.Pin = pin;
  gpio.Mode = mode;
  gpio.Pull = pull;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(port, &gpio);
}

void gpio_config_output(GPIO_TypeDef *port, uint16_t pin, gpio_out_type_t otype,
                        uint32_t pull, gpio_level_t initial)
{
  if (port == NULL) {
    return;
  }

  clk_enable_gpio_port(port);

  /* Preset the output latch before switching to output to avoid a glitch. */
  HAL_GPIO_WritePin(port, pin,
                    (initial == GPIO_LEVEL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  gpio_pin_init(port, pin,
                (otype == GPIO_OUT_OPEN_DRAIN) ? GPIO_MODE_OUTPUT_OD
                                               : GPIO_MODE_OUTPUT_PP,
                pull);
}

void gpio_config_input(GPIO_TypeDef *port, uint16_t pin, uint32_t pull)
{
  if (port == NULL) {
    return;
  }

  gpio_pin_init(port, pin, GPIO_MODE_INPUT, pull);
}

void gpio_write(GPIO_TypeDef *port, uint16_t pin, gpio_level_t level)
{
  HAL_GPIO_WritePin(port, pin,
                    (level == GPIO_LEVEL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void gpio_set(GPIO_TypeDef *port, uint16_t pin)
{
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void gpio_clear(GPIO_TypeDef *port, uint16_t pin)
{
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

void gpio_toggle(GPIO_TypeDef *port, uint16_t pin)
{
  HAL_GPIO_TogglePin(port, pin);
}

gpio_level_t gpio_read(GPIO_TypeDef *port, uint16_t pin)
{
  return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? GPIO_LEVEL_HIGH
                                                       : GPIO_LEVEL_LOW;
}

/* ---- EXTI interrupt dispatch ---- */

#define GPIO_EXTI_LINE_COUNT 16U

typedef struct
{
  gpio_exti_cb_t cb;
  void *ctx;
} gpio_exti_slot_t;

static gpio_exti_slot_t s_exti[GPIO_EXTI_LINE_COUNT];

static int gpio_exti_line_from_pin(uint16_t pin)
{
  for (uint8_t line = 0U; line < GPIO_EXTI_LINE_COUNT; ++line) {
    if (pin == (uint16_t)(1U << line)) {
      return (int)line;
    }
  }
  return -1;
}

static uint32_t gpio_exti_mode(gpio_exti_edge_t edge)
{
  switch (edge) {
  case GPIO_EXTI_EDGE_RISING:
    return GPIO_MODE_IT_RISING;
  case GPIO_EXTI_EDGE_FALLING:
    return GPIO_MODE_IT_FALLING;
  default:
    return GPIO_MODE_IT_RISING_FALLING;
  }
}

int gpio_config_input_exti(GPIO_TypeDef *port, uint16_t pin,
                           gpio_exti_edge_t edge, uint32_t pull,
                           IRQn_Type irqn, uint8_t prio, gpio_exti_cb_t cb,
                           void *ctx)
{
  int line = gpio_exti_line_from_pin(pin);

  if (port == NULL || line < 0 || cb == NULL) {
    return -1;
  }

  /* Register before enabling the line so a first edge always finds a callback. */
  s_exti[line].ctx = ctx;
  s_exti[line].cb = cb;

  /* An EXTI pin is an interrupt input: reuse the shared pin-setup path. */
  gpio_pin_init(port, pin, gpio_exti_mode(edge), pull);

  HAL_NVIC_SetPriority(irqn, prio, 0);
  HAL_NVIC_EnableIRQ(irqn);
  return 0;
}

static void gpio_exti_dispatch(uint16_t pin)
{
  int line = gpio_exti_line_from_pin(pin);

  if (line < 0) {
    return;
  }

  if (s_exti[line].cb != NULL) {
    s_exti[line].cb(pin, s_exti[line].ctx);
  }
}

/* STM32F2 HAL uses the unified EXTI callback for every edge. */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  gpio_exti_dispatch(GPIO_Pin);
}

/* ---- EXTI interrupt vectors ----------------------------------------------
 * The gpio module owns all EXTI vectors (overriding the weak startup symbols)
 * and funnels each into the shared per-line dispatch. A module only registers a
 * callback via gpio_config_input_exti(); it never defines a vector itself.
 * On STM32F2, lines 0..4 have their own vector; lines 5..9 and 10..15 share
 * one vector each (service every line in the group — HAL_GPIO_EXTI_IRQHandler
 * acts only on the line whose pending flag is set). */
void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); }
void EXTI1_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); }
void EXTI2_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2); }
void EXTI3_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3); }
void EXTI4_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4); }

void EXTI9_5_IRQHandler(void)
{
  for (uint8_t line = 5U; line <= 9U; ++line) {
    HAL_GPIO_EXTI_IRQHandler((uint16_t)(1U << line));
  }
}

void EXTI15_10_IRQHandler(void)
{
  for (uint8_t line = 10U; line <= 15U; ++line) {
    HAL_GPIO_EXTI_IRQHandler((uint16_t)(1U << line));
  }
}
