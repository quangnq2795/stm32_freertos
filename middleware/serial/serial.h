#pragma once

#include <stddef.h>
#include <stdint.h>

#include "uart.h"

#define SERIAL_OK        0
#define SERIAL_ERR_PARAM (-1)
#define SERIAL_ERR_BUSY  (-2)

/* Opaque registration handles (valid after successful register). */
typedef struct
{
  uart_id_t uart;
} serial_tx_t;

typedef struct
{
  uart_id_t uart;
} serial_rx_t;

typedef void (*serial_rx_isr_fn_t)(uart_id_t uart, uart_event_t evt, void *ctx);

/*
 * Claim exclusive TX or RX on a UART instance.
 * Returns SERIAL_ERR_BUSY if that direction is already registered.
 * First claim on an instance brings up the hardware (uart_init).
 */
int serial_tx_register(uart_id_t uart, serial_tx_t *out);
int serial_rx_register(uart_id_t uart, serial_rx_isr_fn_t isr_fn, void *ctx,
                       serial_rx_t *out);

void serial_tx_unregister(serial_tx_t *handle);
void serial_rx_unregister(serial_rx_t *handle);

uint8_t serial_tx_is_claimed(uart_id_t uart);
uint8_t serial_rx_is_claimed(uart_id_t uart);

size_t serial_tx_write(const serial_tx_t *handle, const uint8_t *buf, size_t len);
size_t serial_rx_read(const serial_rx_t *handle, uint8_t *buf, size_t max_len);
