#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32f2xx_hal.h"

/* HAL RX chunk size for ReceiveToIdle_IT (driver internal temporary buffer). */
#ifndef UART_RX_IT_CHUNK
#define UART_RX_IT_CHUNK 64U
#endif

/* RX ring buffer size per UART (storage size, max bytes stored = size-1). */
#ifndef UART_RX_RING_SIZE
#define UART_RX_RING_SIZE 256U
#endif

/* UART identifier (numeric ID: 0..BSP_UART_COUNT-1) */
typedef uint8_t uart_id_t;

typedef enum {
    UART_EVENT_RX,
    UART_EVENT_TX_DONE,
    UART_EVENT_ERROR,
} uart_event_t;

/* RX callback is invoked from UART interrupt context (ISR).
 * It is expected to be ISR-safe (e.g. wakeup a FreeRTOS task).
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

  /* Runtime fields (set/used by driver). */
  UART_HandleTypeDef huart;
  uart_event_callback_t evt_cb;
} uart_desc_t;


void   uart_init(uart_id_t id);
size_t uart_read(uart_id_t id, uint8_t *out, size_t max_len);
size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len);
size_t uart_rx_available(uart_id_t id);
void   uart_register_event_callback(uart_id_t id, uart_event_callback_t cb);