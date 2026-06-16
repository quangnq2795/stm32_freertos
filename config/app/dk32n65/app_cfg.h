#pragma once

#include "serial_port.h"

/* STM32N6570-DK: single VCP on USART1 (serial port 1), odd parity in BSP. */

#define CLI_SERIAL_RX     SERIAL_PORT_1_RX
#define LOG_SERIAL_TX     SERIAL_PORT_1_TX
#define CLI_UART_RX_CHUNK 32U
#define LOG_LINE_MAX      128U
