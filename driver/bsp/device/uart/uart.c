#include "uart.h"
#include "bsp_uart_cfg.h"

#include "ringbuf.h"

static uart_desc_t g_uarts[BSP_UART_COUNT] = BSP_UART_DESCS;

static ringbuf_u8_t g_rx_rb[BSP_UART_COUNT];
static uint8_t g_rx_storage[BSP_UART_COUNT][UART_RX_RING_SIZE];
static uint8_t g_rx_tmp[BSP_UART_COUNT][UART_RX_IT_CHUNK];

static void uart_gpio_init(const uart_desc_t *d)
{
  if (d->gpio_clk_enable != 0) {
    d->gpio_clk_enable();
  }

  GPIO_InitTypeDef gpio = {0};
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  // TX
  gpio.Pin = d->tx_pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Alternate = d->tx_af;
  HAL_GPIO_Init(d->tx_port, &gpio);

  // RX
  gpio.Pin = d->rx_pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Alternate = d->rx_af;
  HAL_GPIO_Init(d->rx_port, &gpio);
}

void uart_init(uart_id_t id)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }

  const uart_desc_t *d = &g_uarts[id];

  uart_gpio_init(d);

  /* Initialize per-UART RX ring buffer. */
  ringbuf_init(&g_rx_rb[id], g_rx_storage[id], UART_RX_RING_SIZE);

  g_uarts[id].huart.Instance = d->instance;
  g_uarts[id].huart.Init.BaudRate = d->baudrate;
  g_uarts[id].huart.Init.WordLength = UART_WORDLENGTH_8B;
  g_uarts[id].huart.Init.StopBits = UART_STOPBITS_1;
  g_uarts[id].huart.Init.Parity = UART_PARITY_NONE;
  g_uarts[id].huart.Init.Mode = UART_MODE_TX_RX;
  g_uarts[id].huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  g_uarts[id].huart.Init.OverSampling = UART_OVERSAMPLING_16;

  (void)HAL_UART_Init(&g_uarts[id].huart);

  /* Start RX in interrupt mode using ReceiveToIdle. */
  (void)HAL_UARTEx_ReceiveToIdle_IT(&g_uarts[id].huart,
                                      g_rx_tmp[id],
                                      UART_RX_IT_CHUNK);
}

void uart_register_event_callback(uart_id_t id, uart_event_callback_t cb)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }
  g_uarts[id].evt_cb = cb;
}

size_t uart_rx_available(uart_id_t id)
{
  if (id >= BSP_UART_COUNT) {
    return 0U;
  }

  return ringbuf_len(&g_rx_rb[id]);
}

size_t uart_read(uart_id_t id, uint8_t *out, size_t max_len)
{
  if (id >= BSP_UART_COUNT || out == 0 || max_len == 0U) {
    return 0U;
  }

  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  size_t n = ringbuf_pop(&g_rx_rb[id], out, max_len);
  if (!primask) {
    __enable_irq();
  }
  return n;
}

size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len)
{
  if (id >= BSP_UART_COUNT) {
    return 0U;
  }
  HAL_StatusTypeDef st =
      HAL_UART_Transmit(&g_uarts[id].huart, (uint8_t *)buf, (uint16_t)len, 100);
  if (st == HAL_OK) {
    return len;
  }
  return 0U;
}

/* Called from UART interrupt context when ReceiveToIdle sees new bytes. */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  uart_id_t id = (uart_id_t)BSP_UART_COUNT; /* sentinel: invalid ID */
  for (uart_id_t i = 0U; i < BSP_UART_COUNT; ++i) {
    if (huart == &g_uarts[i].huart) {
      id = i;
      break;
    }
  }
  if (id >= BSP_UART_COUNT) {
    return;
  }

  if (Size == 0) {
    (void)HAL_UARTEx_ReceiveToIdle_IT(&g_uarts[id].huart,
                                        g_rx_tmp[id],
                                        UART_RX_IT_CHUNK);
    return;
  }

  ringbuf_push_isr(&g_rx_rb[id], g_rx_tmp[id], Size);

  if (g_uarts[id].evt_cb != 0) {
    g_uarts[id].evt_cb(id, UART_EVENT_RX);
  }

  /* Restart RX after each event. */
  (void)HAL_UARTEx_ReceiveToIdle_IT(&g_uarts[id].huart,
                                      g_rx_tmp[id],
                                      UART_RX_IT_CHUNK);
}

