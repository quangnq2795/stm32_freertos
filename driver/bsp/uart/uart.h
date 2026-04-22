#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32f2xx_hal.h"

/* HAL ReceiveToIdle_IT temporary buffer length (per UART). */
#ifndef UART_RX_IT_CHUNK
#define UART_RX_IT_CHUNK 64U
#endif

/* RX ring buffer size per UART (storage size, max bytes stored = size-1). */
#ifndef UART_RX_RING_SIZE
#define UART_RX_RING_SIZE 256U
#endif

#ifndef UART_TX_RING_SIZE
#define UART_TX_RING_SIZE 256U
#endif

/* Max bytes per HAL_UART_Transmit_IT chunk (scratch in uart.c). */
#ifndef UART_TX_CHUNK
#define UART_TX_CHUNK 64U
#endif

/* UART identifier (numeric ID: 0..BSP_UART_COUNT-1) */
typedef uint8_t uart_id_t;

typedef enum {
    UART_EVENT_RX_AVAILABLE,
    UART_EVENT_RX_OVERFLOW,
    UART_EVENT_TX_EMPTY,
    UART_EVENT_ERROR,
} uart_event_t;

/* Callback is invoked from UART interrupt context (ISR).
 * It is expected to be ISR-safe (e.g. wakeup a FreeRTOS task).
 * UART_EVENT_RX_OVERFLOW: fewer bytes stored in RX ring than received (ring full).
 */
typedef void (*uart_event_callback_t)(uart_id_t id, uart_event_t event);

typedef struct
{
  /* Board/config fields (static). */
  USART_TypeDef *instance;
  uint32_t baudrate;

  GPIO_TypeDef *tx_port;
  uint16_t tx_pin;
  uint8_t tx_af;

  GPIO_TypeDef *rx_port;
  uint16_t rx_pin;
  uint8_t rx_af;

  void (*gpio_clk_enable)(void);
  void (*uart_clk_enable)(void);
  IRQn_Type irqn;

  /* Runtime fields (set/used by driver). */
  UART_HandleTypeDef huart;
  uart_event_callback_t evt_cb;
} uart_desc_t;


void   uart_init(uart_id_t id);
size_t uart_read(uart_id_t id, uint8_t *out, size_t max_len);
/* Enqueue up to len bytes into TX ring; returns count actually queued (may be < len). */
size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len);
size_t uart_rx_available(uart_id_t id);
void   uart_register_event_callback(uart_id_t id, uart_event_callback_t cb);