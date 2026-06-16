#pragma once

#include "serial_port.h"

/* NUCLEO-F207ZG: CLI + log on ST-Link VCP (USART3 = serial port 2). */

#define CLI_SERIAL_RX     SERIAL_PORT_2_RX
#define LOG_SERIAL_TX     SERIAL_PORT_2_TX
#define CLI_UART_RX_CHUNK 32U
#define LOG_LINE_MAX      128U
