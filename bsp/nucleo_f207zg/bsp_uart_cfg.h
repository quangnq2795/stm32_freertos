#pragma once

#include "stm32_hal.h"

#define BSP_UART_COUNT 2U

/* Logical UART ports on this board = index into BSP_UART_DESCS (uart_id_t).
 * Use these names (not raw 0/1) when selecting a port, e.g. in bsp_common_cfg.h.
 * Keep BSP_UART_COUNT as a macro: serial_port.h evaluates it in #if. */
typedef enum
{
  BSP_UART_USART1 = 0,
  BSP_UART_USART3 = 1,
} bsp_uart_port_t;

/* BSP_UART3 = ST-Link Virtual COM (USART3 on PD8/PD9). */

/* Map generic UART indices to this board's physical UARTs. */
#define BSP_UART1_INSTANCE USART1
#define BSP_UART1_BAUDRATE 115200U
#define BSP_UART1_PARITY UART_PARITY_NONE
#define BSP_UART1_TX_GPIO_PORT GPIOA
#define BSP_UART1_TX_GPIO_PIN  GPIO_PIN_9
#define BSP_UART1_TX_AF GPIO_AF7_USART1
#define BSP_UART1_RX_GPIO_PORT GPIOA
#define BSP_UART1_RX_GPIO_PIN  GPIO_PIN_10
#define BSP_UART1_RX_AF GPIO_AF7_USART1
#define BSP_UART1_IRQN USART1_IRQn

#define BSP_UART3_INSTANCE USART3
#define BSP_UART3_BAUDRATE 115200U
#define BSP_UART3_PARITY UART_PARITY_NONE
#define BSP_UART3_TX_GPIO_PORT GPIOD
#define BSP_UART3_TX_GPIO_PIN  GPIO_PIN_8
#define BSP_UART3_TX_AF GPIO_AF7_USART3
#define BSP_UART3_RX_GPIO_PORT GPIOD
#define BSP_UART3_RX_GPIO_PIN  GPIO_PIN_9
#define BSP_UART3_RX_AF GPIO_AF7_USART3
#define BSP_UART3_IRQN USART3_IRQn

#ifndef BSP_UART_IRQ_HANDLER
#define BSP_UART_IRQ_HANDLER(USARTn) \
  void USARTn##_IRQHandler(void) { uart_irq_handler(USARTn); }
#endif
/* Mở rộng trong một file .c (vd. uart.c): BSP_UART_IRQ_HANDLERS */
#define BSP_UART_IRQ_HANDLERS \
  BSP_UART_IRQ_HANDLER(USART1) \
  BSP_UART_IRQ_HANDLER(USART3)

/* Initializer for generic UART driver. */
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
    [BSP_UART_USART3] = {                                                   \
      .instance = BSP_UART3_INSTANCE,                                      \
      .baudrate = BSP_UART3_BAUDRATE,                                      \
      .parity = BSP_UART3_PARITY,                                          \
      .tx_port = BSP_UART3_TX_GPIO_PORT,                                  \
      .tx_pin = BSP_UART3_TX_GPIO_PIN,                                    \
      .tx_af = BSP_UART3_TX_AF,                                            \
      .rx_port = BSP_UART3_RX_GPIO_PORT,                                  \
      .rx_pin = BSP_UART3_RX_GPIO_PIN,                                    \
      .rx_af = BSP_UART3_RX_AF,                                            \
      .irqn = BSP_UART3_IRQN,                                              \
    },                                                                      \
  }

