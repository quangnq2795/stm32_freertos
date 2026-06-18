#pragma once

#include "stm32_hal.h"

#define BSP_UART_COUNT 1U

/* ST-Link Virtual COM on STM32N6570-DK: USART1 PE5/PE6, odd parity. */

#define BSP_UART1_INSTANCE USART1
#define BSP_UART1_BAUDRATE 115200U
#define BSP_UART1_PARITY UART_PARITY_NONE
#define BSP_UART1_TX_GPIO_PORT GPIOE
#define BSP_UART1_TX_GPIO_PIN  GPIO_PIN_5
#define BSP_UART1_TX_AF GPIO_AF7_USART1
#define BSP_UART1_RX_GPIO_PORT GPIOE
#define BSP_UART1_RX_GPIO_PIN  GPIO_PIN_6
#define BSP_UART1_RX_AF GPIO_AF7_USART1
#define BSP_UART1_IRQN USART1_IRQn

#ifndef BSP_UART_IRQ_HANDLER
#define BSP_UART_IRQ_HANDLER(USARTn) \
  void USARTn##_IRQHandler(void) { uart_irq_handler(USARTn); }
#endif

#define BSP_UART_IRQ_HANDLERS \
  BSP_UART_IRQ_HANDLER(USART1)

#define BSP_UART_DESCS                                                     \
  {                                                                          \
    [0] = {                                                                 \
      .instance = BSP_UART1_INSTANCE,                                      \
      .baudrate = BSP_UART1_BAUDRATE,                                      \
      .parity = BSP_UART1_PARITY,                                          \
      .tx_port = BSP_UART1_TX_GPIO_PORT,                                  \
      .tx_pin = BSP_UART1_TX_GPIO_PIN,                                    \
      .tx_af = BSP_UART1_TX_AF,                                            \
      .rx_port = BSP_UART1_RX_GPIO_PORT,                                  \
      .rx_pin = BSP_UART1_RX_GPIO_PIN,                                    \
      .rx_af = BSP_UART1_RX_AF,                                            \
      .irqn = BSP_UART1_IRQN,                                              \
    },                                                                      \
  }
