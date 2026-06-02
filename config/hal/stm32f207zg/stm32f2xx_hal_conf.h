#ifndef __STM32F2xx_HAL_CONF_H
#define __STM32F2xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

#define HSE_VALUE    ((uint32_t)8000000U)
#define HSE_STARTUP_TIMEOUT    ((uint32_t)100U)
#define HSI_VALUE    ((uint32_t)16000000U)
#define LSI_VALUE    ((uint32_t)32000U)
#define LSE_VALUE    ((uint32_t)32768U)
#define LSE_STARTUP_TIMEOUT ((uint32_t)5000U)
#define EXTERNAL_CLOCK_VALUE    ((uint32_t)12288000U)

#define VDD_VALUE                    ((uint32_t)3300U)
#define TICK_INT_PRIORITY            ((uint32_t)0x0FU)
#define USE_RTOS                     0U
#define PREFETCH_ENABLE              1U
#define INSTRUCTION_CACHE_ENABLE     1U
#define DATA_CACHE_ENABLE            1U

#define USE_HAL_UART_REGISTER_CALLBACKS        0U
#define USE_HAL_RCC_REGISTER_CALLBACKS         0U

#define assert_param(expr) ((void)0U)

#include "stm32f2xx_hal_rcc.h"
#include "stm32f2xx_hal_cortex.h"
#include "stm32f2xx_hal_gpio.h"
#include "stm32f2xx_hal_dma.h"
#include "stm32f2xx_hal_flash.h"
#include "stm32f2xx_hal_uart.h"

#ifdef __cplusplus
}
#endif

#endif
