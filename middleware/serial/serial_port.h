#pragma once

#include "uart.h"
#include "bsp_uart_cfg.h"

/*
 * Logical serial ports (1-based), mapped to uart_id_t in BSP_UART_DESCS order.
 * Each UART provides one TX and one RX endpoint (same underlying instance).
 */

#if (BSP_UART_COUNT >= 1U)
#define SERIAL_PORT_1_TX  ((uart_id_t)0U)
#define SERIAL_PORT_1_RX  ((uart_id_t)0U)
#endif

#if (BSP_UART_COUNT >= 2U)
#define SERIAL_PORT_2_TX  ((uart_id_t)1U)
#define SERIAL_PORT_2_RX  ((uart_id_t)1U)
#endif

#if (BSP_UART_COUNT >= 3U)
#define SERIAL_PORT_3_TX  ((uart_id_t)2U)
#define SERIAL_PORT_3_RX  ((uart_id_t)2U)
#endif

#if (BSP_UART_COUNT >= 4U)
#define SERIAL_PORT_4_TX  ((uart_id_t)3U)
#define SERIAL_PORT_4_RX  ((uart_id_t)3U)
#endif

#if (BSP_UART_COUNT >= 5U)
#define SERIAL_PORT_5_TX  ((uart_id_t)4U)
#define SERIAL_PORT_5_RX  ((uart_id_t)4U)
#endif

#if (BSP_UART_COUNT >= 6U)
#define SERIAL_PORT_6_TX  ((uart_id_t)5U)
#define SERIAL_PORT_6_RX  ((uart_id_t)5U)
#endif

#if (BSP_UART_COUNT >= 7U)
#define SERIAL_PORT_7_TX  ((uart_id_t)6U)
#define SERIAL_PORT_7_RX  ((uart_id_t)6U)
#endif

#if (BSP_UART_COUNT >= 8U)
#define SERIAL_PORT_8_TX  ((uart_id_t)7U)
#define SERIAL_PORT_8_RX  ((uart_id_t)7U)
#endif

#if (BSP_UART_COUNT > 8U)
#error "Extend serial_port.h when BSP_UART_COUNT > 8"
#endif

#define SERIAL_PORT_COUNT  BSP_UART_COUNT
