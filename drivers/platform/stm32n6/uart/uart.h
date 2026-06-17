#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32_hal.h"

/* HAL ReceiveToIdle_IT temporary buffer length (per UART). */
#ifndef UART_RX_IT_CHUNK
#define UART_RX_IT_CHUNK 64U
#endif

/* RX ring buffer size per UART (storage size, max bytes stored = size-1). */
#ifndef UART_RX_RING_SIZE
#define UART_RX_RING_SIZE 256U
#endif

/* HAL Transmit_IT in-flight chunk staging length (per UART). */
#ifndef UART_TX_IT_CHUNK
#define UART_TX_IT_CHUNK 64U
#endif

/* TX ring buffer size per UART (storage size, max bytes queued = size-1). */
#ifndef UART_TX_RING_SIZE
#define UART_TX_RING_SIZE 256U
#endif

/* UART identifier (numeric ID: 0..BSP_UART_COUNT-1) */
typedef uint8_t uart_id_t;

typedef enum
{
  UART_EVENT_RX_AVAILABLE,
  UART_EVENT_TX_EMPTY,
} uart_event_t;

/* Callback is invoked from UART interrupt context (ISR).
 * Must be ISR-safe (e.g. FreeRTOS FromISR APIs only).
 */
typedef void (*uart_event_callback_t)(uart_id_t id, uart_event_t event);

typedef struct
{
  USART_TypeDef *instance;
  uint32_t baudrate;
  uint32_t parity;

  GPIO_TypeDef *tx_port;
  uint16_t tx_pin;
  uint8_t tx_af;

  GPIO_TypeDef *rx_port;
  uint16_t rx_pin;
  uint8_t rx_af;

  IRQn_Type irqn;

  UART_HandleTypeDef huart;
  uart_event_callback_t evt_cb;
} uart_desc_t;

void uart_init(uart_id_t id);
size_t uart_read(uart_id_t id, uint8_t *out, size_t max_len);

/* Copy up to `len` bytes into the TX ring and (re)start interrupt-driven
 * transmit. Returns bytes accepted (may be < len if the ring is full, 0 if
 * none fit). The caller's buffer is copied, so it need not outlive the call.
 * UART_EVENT_TX_EMPTY fires (ISR context) once the ring has fully drained.
 * Single-producer: only one context may write a given UART. */
size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len);
void uart_register_event(uart_id_t id, uart_event_callback_t cb);
