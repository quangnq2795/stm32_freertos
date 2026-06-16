#ifndef STM32N6xx_HAL_CONF_H
#define STM32N6xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_EXTI_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED

#define HSE_VALUE               48000000UL
#define HSE_STARTUP_TIMEOUT     100UL
#define HSI_VALUE               64000000UL
#define LSI_VALUE               32000UL
#define LSE_VALUE               32768UL
#define LSE_STARTUP_TIMEOUT     5000UL
#define EXTERNAL_CLOCK_VALUE    12288000UL

#define VDD_VALUE               3300U
#define TICK_INT_PRIORITY       15U
#define USE_RTOS                0U
#define PREFETCH_ENABLE         0U
#define INSTRUCTION_CACHE_ENABLE 0U
#define DATA_CACHE_ENABLE       0U

#define USE_HAL_UART_REGISTER_CALLBACKS 0U
#define USE_HAL_RCC_REGISTER_CALLBACKS  0U

#define assert_param(expr) ((void)0U)

#include "stm32n6xx_hal_rcc.h"
#include "stm32n6xx_hal_cortex.h"
#include "stm32n6xx_hal_gpio.h"
#include "stm32n6xx_hal_dma.h"
#include "stm32n6xx_hal_pwr.h"
#include "stm32n6xx_hal_uart.h"
#include "stm32n6xx_hal_tim.h"
#include "stm32n6xx_hal_i2c.h"
#include "stm32n6xx_hal_exti.h"

#ifdef __cplusplus
}
#endif

#endif /* STM32N6xx_HAL_CONF_H */
