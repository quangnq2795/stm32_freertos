#pragma once

#include "bsp_uart_cfg.h"

/*
 * Board-level wiring of console services to physical UART ports.
 * The TX/RX direction is chosen when the service registers; here we only pick
 * which UART each one uses (see bsp_uart_port_t in bsp_uart_cfg.h).
 *
 * NUCLEO-F207ZG: CLI + log on the ST-Link VCP (USART3).
 */
#define CLI_PORT  BSP_UART_USART3
#define LOG_PORT  BSP_UART_USART3
