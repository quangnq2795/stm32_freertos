#include "serial.h"

#include "bsp_uart_cfg.h"

static uint8_t s_tx_claimed[BSP_UART_COUNT];
static uint8_t s_rx_claimed[BSP_UART_COUNT];
static uint8_t s_hw_up[BSP_UART_COUNT];

static serial_rx_isr_fn_t s_rx_isr[BSP_UART_COUNT];
static void *s_rx_ctx[BSP_UART_COUNT];

static void serial_uart_evt_cb(uart_id_t id, uart_event_t evt)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }

  if (s_rx_isr[id] != NULL) {
    s_rx_isr[id](id, evt, s_rx_ctx[id]);
  }
}

static int serial_ensure_hw(uart_id_t uart)
{
  if (uart >= BSP_UART_COUNT) {
    return SERIAL_ERR_PARAM;
  }

  if (s_hw_up[uart] == 0U) {
    uart_init(uart);
    s_hw_up[uart] = 1U;
  }

  return SERIAL_OK;
}

static uint8_t serial_claimed(uart_id_t port, serial_type_t type)
{
  if (port >= BSP_UART_COUNT) {
    return 0U;
  }

  if (type == SERIAL_TYPE_TX) {
    return s_tx_claimed[port];
  }

  return s_rx_claimed[port];
}

int serial_register(uart_id_t port, serial_type_t type, const serial_cfg_t *cfg,
                    serial_t *out)
{
  if (port >= BSP_UART_COUNT || out == NULL) {
    return SERIAL_ERR_PARAM;
  }

  if (type == SERIAL_TYPE_RX && cfg == NULL) {
    return SERIAL_ERR_PARAM;
  }

  if (serial_claimed(port, type) != 0U) {
    return SERIAL_ERR_BUSY;
  }

  if (serial_ensure_hw(port) != SERIAL_OK) {
    return SERIAL_ERR_PARAM;
  }

  if (type == SERIAL_TYPE_TX) {
    s_tx_claimed[port] = 1U;
  } else {
    s_rx_claimed[port] = 1U;
    s_rx_isr[port] = cfg->isr_fn;
    s_rx_ctx[port] = cfg->ctx;
    uart_register_event(port, serial_uart_evt_cb);
  }

  out->port = port;
  out->type = type;
  return SERIAL_OK;
}

void serial_unregister(serial_t *handle)
{
  if (handle == NULL || handle->port >= BSP_UART_COUNT) {
    return;
  }

  if (handle->type == SERIAL_TYPE_TX) {
    s_tx_claimed[handle->port] = 0U;
  } else {
    uart_id_t port = handle->port;

    s_rx_claimed[port] = 0U;
    s_rx_isr[port] = NULL;
    s_rx_ctx[port] = NULL;
    uart_register_event(port, NULL);
  }
}

size_t serial_write(const serial_t *handle, const uint8_t *buf, size_t len)
{
  if (handle == NULL || buf == NULL || len == 0U) {
    return 0U;
  }

  if (handle->type != SERIAL_TYPE_TX ||
      handle->port >= BSP_UART_COUNT ||
      s_tx_claimed[handle->port] == 0U) {
    return 0U;
  }

  return uart_write(handle->port, buf, len);
}

size_t serial_read(const serial_t *handle, uint8_t *buf, size_t max_len)
{
  if (handle == NULL || buf == NULL || max_len == 0U) {
    return 0U;
  }

  if (handle->type != SERIAL_TYPE_RX ||
      handle->port >= BSP_UART_COUNT ||
      s_rx_claimed[handle->port] == 0U) {
    return 0U;
  }

  return uart_read(handle->port, buf, max_len);
}
