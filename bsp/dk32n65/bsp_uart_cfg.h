#pragma once

#include "stm32_hal.h"

#define BSP_UART_COUNT 1U

/* Logical UART ports on this board = index into BSP_UART_DESCS (uart_id_t).
 * Use these names (not raw 0/1) when selecting a port, e.g. in bsp_common_cfg.h.
 * Keep BSP_UART_COUNT as a macro: serial_port.h evaluates it in #if. */
typedef enum
{
  BSP_UART_USART1 = 0,
} bsp_uart_port_t;

/* ST-Link Virtual COM on STM32N6570-DK: USART1 PE5/PE6, 8N1. */

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
    [BSP_UART_USART1] = {                                                   \
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
