#pragma once

#include "serial_port.h"

/* STM32N6570-DK: single VCP on USART1 (serial port 1), 8N1. */

#define CLI_SERIAL_RX     SERIAL_PORT_1_RX
#define LOG_SERIAL_TX     SERIAL_PORT_1_TX
