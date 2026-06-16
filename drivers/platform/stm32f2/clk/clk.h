#pragma once

#include "stm32_hal.h"

/* SYSCLK/bus from boards_clk_cfg.h (PLL + HSE or HSI). */
void clk_init(void);

/* GPIO / peripheral clocks used by platform drivers. */
void clk_enable_gpio_port(GPIO_TypeDef *port);
void clk_enable_uart(USART_TypeDef *instance);
void clk_disable_uart(USART_TypeDef *instance);
int clk_config_uart_source(USART_TypeDef *instance);
void clk_enable_i2c(I2C_TypeDef *instance);
void clk_disable_i2c(I2C_TypeDef *instance);
int clk_config_i2c_source(I2C_TypeDef *instance);
void clk_enable_tim(TIM_TypeDef *instance);
void clk_disable_tim(TIM_TypeDef *instance);
