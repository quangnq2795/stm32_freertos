#pragma once

#include <stddef.h>
#include <stdint.h>

#include "serial_port.h"
#include "uart.h"

#define SERIAL_OK        0
#define SERIAL_ERR_PARAM (-1)
#define SERIAL_ERR_BUSY  (-2)

typedef enum
{
  SERIAL_TYPE_TX,
  SERIAL_TYPE_RX,
} serial_type_t;

typedef void (*serial_rx_isr_fn_t)(uart_id_t port, uart_event_t evt, void *ctx);

typedef struct
{
  uart_id_t port;
  serial_type_t type;
} serial_t;

typedef struct
{
  serial_rx_isr_fn_t isr_fn;
  void *ctx;
} serial_cfg_t;

/*
 * Claim exclusive TX or RX on a serial port (SERIAL_PORT_N_TX / SERIAL_PORT_N_RX).
 * cfg: required (isr_fn may be NULL; TX uses isr_fn for UART_EVENT_TX_EMPTY).
 * Returns SERIAL_ERR_BUSY if that direction is already registered.
 */
int serial_register(uart_id_t port, serial_type_t type, const serial_cfg_t *cfg,
                    serial_t *out);
void serial_unregister(serial_t *handle);

size_t serial_write(const serial_t *handle, const uint8_t *buf, size_t len);
size_t serial_read(const serial_t *handle, uint8_t *buf, size_t max_len);
