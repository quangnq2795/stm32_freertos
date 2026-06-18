#pragma once

#include <stdint.h>

#include "stm32_hal.h"

/*
 * GPIO platform driver: shared EXTI dispatcher.
 *
 * Owns the single HAL_GPIO_EXTI_Callback for the whole program and routes each
 * EXTI line (0..15) to a per-line callback registered by a module. This lets
 * several modules use GPIO interrupts without fighting over the weak HAL
 * callback.
 *
 * One EXTI line maps to one pin number across all ports (hardware limit), so a
 * given line has a single owner.
 */

typedef enum
{
  GPIO_LEVEL_LOW = 0,
  GPIO_LEVEL_HIGH,
} gpio_level_t;

typedef enum
{
  GPIO_OUT_PUSH_PULL = 0,
  GPIO_OUT_OPEN_DRAIN,
} gpio_out_type_t;

typedef enum
{
  GPIO_EXTI_EDGE_RISING = 0,
  GPIO_EXTI_EDGE_FALLING,
  GPIO_EXTI_EDGE_BOTH,
} gpio_exti_edge_t;

/* ---- Basic GPIO control ---- */

/* Configure `pin` as an output (enables the port clock) and drive `initial`.
 * The level is latched before the pin starts driving to avoid a startup glitch. */
void gpio_config_output(GPIO_TypeDef *port, uint16_t pin, gpio_out_type_t otype,
                        uint32_t pull, gpio_level_t initial);

/* Configure `pin` as a plain input (enables the port clock). */
void gpio_config_input(GPIO_TypeDef *port, uint16_t pin, uint32_t pull);

void gpio_write(GPIO_TypeDef *port, uint16_t pin, gpio_level_t level);
void gpio_set(GPIO_TypeDef *port, uint16_t pin);
void gpio_clear(GPIO_TypeDef *port, uint16_t pin);
void gpio_toggle(GPIO_TypeDef *port, uint16_t pin);
gpio_level_t gpio_read(GPIO_TypeDef *port, uint16_t pin);

/* ---- EXTI interrupt dispatch ---- */

/* Invoked from EXTI interrupt context. `pin` is a single GPIO_PIN_x bit. */
typedef void (*gpio_exti_cb_t)(uint16_t pin, void *ctx);

/*
 * Configure `pin` as an interrupt input and register `cb` for its EXTI line
 * (the interrupt-mode counterpart of gpio_config_input): enables the port
 * clock, sets edge + pull, sets the NVIC priority and enables `irqn`.
 * Returns 0 on success, -1 on invalid arguments.
 */
int gpio_config_input_exti(GPIO_TypeDef *port, uint16_t pin,
                           gpio_exti_edge_t edge, uint32_t pull,
                           IRQn_Type irqn, uint8_t prio, gpio_exti_cb_t cb,
                           void *ctx);
