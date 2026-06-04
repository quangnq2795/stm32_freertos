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

int serial_tx_register(uart_id_t uart, serial_tx_t *out)
{
  if (uart >= BSP_UART_COUNT || out == NULL) {
    return SERIAL_ERR_PARAM;
  }

  if (s_tx_claimed[uart] != 0U) {
    return SERIAL_ERR_BUSY;
  }

  if (serial_ensure_hw(uart) != SERIAL_OK) {
    return SERIAL_ERR_PARAM;
  }

  s_tx_claimed[uart] = 1U;
  out->uart = uart;
  return SERIAL_OK;
}

int serial_rx_register(uart_id_t uart, serial_rx_isr_fn_t isr_fn, void *ctx,
                       serial_rx_t *out)
{
  if (uart >= BSP_UART_COUNT || out == NULL) {
    return SERIAL_ERR_PARAM;
  }

  if (s_rx_claimed[uart] != 0U) {
    return SERIAL_ERR_BUSY;
  }

  if (serial_ensure_hw(uart) != SERIAL_OK) {
    return SERIAL_ERR_PARAM;
  }

  s_rx_claimed[uart] = 1U;
  s_rx_isr[uart] = isr_fn;
  s_rx_ctx[uart] = ctx;
  uart_register_event(uart, serial_uart_evt_cb);
  out->uart = uart;
  return SERIAL_OK;
}

void serial_tx_unregister(serial_tx_t *handle)
{
  if (handle == NULL || handle->uart >= BSP_UART_COUNT) {
    return;
  }

  s_tx_claimed[handle->uart] = 0U;
}

void serial_rx_unregister(serial_rx_t *handle)
{
  if (handle == NULL || handle->uart >= BSP_UART_COUNT) {
    return;
  }

  uart_id_t uart = handle->uart;

  s_rx_claimed[uart] = 0U;
  s_rx_isr[uart] = NULL;
  s_rx_ctx[uart] = NULL;
  uart_register_event(uart, NULL);
}

uint8_t serial_tx_is_claimed(uart_id_t uart)
{
  if (uart >= BSP_UART_COUNT) {
    return 0U;
  }

  return s_tx_claimed[uart];
}

uint8_t serial_rx_is_claimed(uart_id_t uart)
{
  if (uart >= BSP_UART_COUNT) {
    return 0U;
  }

  return s_rx_claimed[uart];
}

size_t serial_tx_write(const serial_tx_t *handle, const uint8_t *buf, size_t len)
{
  if (handle == NULL || buf == NULL || len == 0U) {
    return 0U;
  }

  if (handle->uart >= BSP_UART_COUNT ||
      s_tx_claimed[handle->uart] == 0U) {
    return 0U;
  }

  return uart_write(handle->uart, buf, len);
}

size_t serial_rx_read(const serial_rx_t *handle, uint8_t *buf, size_t max_len)
{
  if (handle == NULL || buf == NULL || max_len == 0U) {
    return 0U;
  }

  if (handle->uart >= BSP_UART_COUNT ||
      s_rx_claimed[handle->uart] == 0U) {
    return 0U;
  }

  return uart_read(handle->uart, buf, max_len);
}
