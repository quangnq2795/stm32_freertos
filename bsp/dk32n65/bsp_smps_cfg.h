#pragma once

#include "stm32_hal.h"

/*
 * External SMPS voltage-select control pin on the STM32N6570-DK.
 *
 * The board's step-down converter feeds VDDCORE; this pin selects its output:
 *   HIGH = overdrive  (required before raising the core to 800 MHz)
 *   LOW  = nominal
 *
 * Replaces BSP_SMPS_Init() from the STM32Cube BSP (which drove the same PF4).
 */
#define BSP_SMPS_GPIO_PORT GPIOF
#define BSP_SMPS_GPIO_PIN  GPIO_PIN_4
