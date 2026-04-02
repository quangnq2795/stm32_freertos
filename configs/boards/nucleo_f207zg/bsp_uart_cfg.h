#pragma once

#define BSP_UART_COUNT 2U

/* Map generic UART indices to this board's physical UARTs. */
#define BSP_UART1_INSTANCE USART1
#define BSP_UART1_BAUDRATE 115200U
#define BSP_UART1_TX_GPIO_PORT GPIOA
#define BSP_UART1_TX_GPIO_PIN  GPIO_PIN_9
#define BSP_UART1_TX_AF GPIO_AF7_USART1
#define BSP_UART1_RX_GPIO_PORT GPIOA
#define BSP_UART1_RX_GPIO_PIN  GPIO_PIN_10
#define BSP_UART1_RX_AF GPIO_AF7_USART1
#define BSP_UART1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BSP_UART1_CLK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()

#define BSP_UART2_INSTANCE USART2
#define BSP_UART2_BAUDRATE 115200U
#define BSP_UART2_TX_GPIO_PORT GPIOA
#define BSP_UART2_TX_GPIO_PIN  GPIO_PIN_2
#define BSP_UART2_TX_AF GPIO_AF7_USART2
#define BSP_UART2_RX_GPIO_PORT GPIOA
#define BSP_UART2_RX_GPIO_PIN  GPIO_PIN_3
#define BSP_UART2_RX_AF GPIO_AF7_USART2
#define BSP_UART2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BSP_UART2_CLK_ENABLE() __HAL_RCC_USART2_CLK_ENABLE()

/* Helper macro: generate a small wrapper around GPIO/UART clock macros.
 * Note: pass the macro name without trailing `()` (e.g. BSP_UART2_CLK_ENABLE),
 * because wrapper calls it as `macro();`.
 */
#define DEFINE_UART_CLK_ENABLE_FUNC(name, macro) \
  static inline void name(void) { macro(); }

DEFINE_UART_CLK_ENABLE_FUNC(uart1_gpio_clk_enable, BSP_UART1_GPIO_CLK_ENABLE)
DEFINE_UART_CLK_ENABLE_FUNC(uart1_uart_clk_enable, BSP_UART1_CLK_ENABLE)
DEFINE_UART_CLK_ENABLE_FUNC(uart2_gpio_clk_enable, BSP_UART2_GPIO_CLK_ENABLE)
DEFINE_UART_CLK_ENABLE_FUNC(uart2_uart_clk_enable, BSP_UART2_CLK_ENABLE)

/* Initializer for generic UART driver. */
#define BSP_UART_DESCS                                                     \
  {                                                                          \
    [0] = {                                                                 \
      .instance = BSP_UART1_INSTANCE,                                      \
      .baudrate = BSP_UART1_BAUDRATE,                                      \
      .tx_port = BSP_UART1_TX_GPIO_PORT,                                  \
      .tx_pin = BSP_UART1_TX_GPIO_PIN,                                    \
      .tx_af = BSP_UART1_TX_AF,                                            \
      .rx_port = BSP_UART1_RX_GPIO_PORT,                                  \
      .rx_pin = BSP_UART1_RX_GPIO_PIN,                                    \
      .rx_af = BSP_UART1_RX_AF,                                            \
      .gpio_clk_enable = uart1_gpio_clk_enable,                            \
      .uart_clk_enable = uart1_uart_clk_enable,                            \
    },                                                                      \
    [1] = {                                                                 \
      .instance = BSP_UART2_INSTANCE,                                      \
      .baudrate = BSP_UART2_BAUDRATE,                                      \
      .tx_port = BSP_UART2_TX_GPIO_PORT,                                  \
      .tx_pin = BSP_UART2_TX_GPIO_PIN,                                    \
      .tx_af = BSP_UART2_TX_AF,                                            \
      .rx_port = BSP_UART2_RX_GPIO_PORT,                                  \
      .rx_pin = BSP_UART2_RX_GPIO_PIN,                                    \
      .rx_af = BSP_UART2_RX_AF,                                            \
      .gpio_clk_enable = uart2_gpio_clk_enable,                            \
      .uart_clk_enable = uart2_uart_clk_enable,                            \
    },                                                                      \
  }

