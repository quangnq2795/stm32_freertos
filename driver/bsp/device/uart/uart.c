#include "uart.h"
#include "bsp_uart_cfg.h"

#include "ringbuf.h"

static uart_desc_t g_uarts[BSP_UART_COUNT] = BSP_UART_DESCS;
static ringbuf_u8_t g_rx_rb[BSP_UART_COUNT];
static uint8_t g_rx_storage[BSP_UART_COUNT][UART_RX_RING_SIZE];
static uint8_t g_rx_tmp[BSP_UART_COUNT][UART_RX_IT_CHUNK];

static ringbuf_u8_t g_tx_rb[BSP_UART_COUNT];
static uint8_t g_tx_storage[BSP_UART_COUNT][UART_TX_RING_SIZE];
static uint8_t g_tx_chunk[BSP_UART_COUNT][UART_TX_CHUNK];

static uart_id_t uart_id_from_huart(const UART_HandleTypeDef *huart)
{
  for (uart_id_t i = 0U; i < BSP_UART_COUNT; ++i) {
    if (huart == &g_uarts[i].huart) {
      return i;
    }
  }
  return (uart_id_t)BSP_UART_COUNT;
}

static void uart_tx_try_start(uart_id_t id)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }

  UART_HandleTypeDef *const hu = &g_uarts[id].huart;
  if (hu->gState != HAL_UART_STATE_READY) {
    return;
  }
  if (ringbuf_len(&g_tx_rb[id]) == 0U) {
    return;
  }

  size_t n = ringbuf_pop(&g_tx_rb[id], g_tx_chunk[id], UART_TX_CHUNK);
  if (n == 0U) {
    return;
  }

  (void)HAL_UART_Transmit_IT(hu, g_tx_chunk[id], (uint16_t)n);
}

static void uart_rx_try_start(uart_id_t id)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }

  (void)HAL_UARTEx_ReceiveToIdle_IT(&g_uarts[id].huart,
                                      g_rx_tmp[id],
                                      UART_RX_IT_CHUNK);
}

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

  /* RX: AF + pull-up (idle RX high on TTL/USB-UART). */
  gpio.Pin = d->rx_pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Alternate = d->rx_af;
  HAL_GPIO_Init(d->rx_port, &gpio);
}

void uart_irq_handler(USART_TypeDef *instance)
{
  if (instance == NULL) {
    return;
  }
  for (uart_id_t i = 0U; i < BSP_UART_COUNT; ++i) {
    if (g_uarts[i].huart.Instance == instance) {
      HAL_UART_IRQHandler(&g_uarts[i].huart);
      return;
    }
  }
}

void uart_init(uart_id_t id)
{
  if (id >= BSP_UART_COUNT) {
    return;
  }

  const uart_desc_t *d = &g_uarts[id];

  uart_gpio_init(d);

  if (d->uart_clk_enable != NULL) {
    d->uart_clk_enable();
  }
  if ((int32_t)d->irqn >= 0) {
    /* Priority >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) for FreeRTOS ISR API. */
    HAL_NVIC_SetPriority(d->irqn, 6, 0);
    HAL_NVIC_EnableIRQ(d->irqn);
  }

  ringbuf_init(&g_rx_rb[id], g_rx_storage[id], UART_RX_RING_SIZE);
  ringbuf_init(&g_tx_rb[id], g_tx_storage[id], UART_TX_RING_SIZE);

  g_uarts[id].huart.Instance = d->instance;
  g_uarts[id].huart.Init.BaudRate = d->baudrate;
  g_uarts[id].huart.Init.WordLength = UART_WORDLENGTH_8B;
  g_uarts[id].huart.Init.StopBits = UART_STOPBITS_1;
  g_uarts[id].huart.Init.Parity = UART_PARITY_NONE;
  g_uarts[id].huart.Init.Mode = UART_MODE_TX_RX;
  g_uarts[id].huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  g_uarts[id].huart.Init.OverSampling = UART_OVERSAMPLING_16;

  (void)HAL_UART_Init(&g_uarts[id].huart);

  uart_rx_try_start(id);
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

  IRQn_Type const irqn = g_uarts[id].irqn;
  if ((int32_t)irqn >= 0) {
    NVIC_DisableIRQ(irqn);
  }

  size_t n = ringbuf_pop(&g_rx_rb[id], out, max_len);

  if ((int32_t)irqn >= 0) {
    NVIC_EnableIRQ(irqn);
  }
  return n;
}

size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len)
{
  if (id >= BSP_UART_COUNT || buf == NULL || len == 0U) {
    return 0U;
  }

  IRQn_Type const irqn = g_uarts[id].irqn;
  if ((int32_t)irqn >= 0) {
    NVIC_DisableIRQ(irqn);
  }
  size_t pushed = ringbuf_push(&g_tx_rb[id], buf, len);
  uart_tx_try_start(id);

  if ((int32_t)irqn >= 0) {
    NVIC_EnableIRQ(irqn);
  }
  return pushed;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);
  if (id >= BSP_UART_COUNT) {
    return;
  }

  uart_tx_try_start(id);

  /* If nothing left in TX ring after completing last chunk, notify. */
  if (g_uarts[id].evt_cb != NULL && ringbuf_len(&g_tx_rb[id]) == 0U
      && huart->gState == HAL_UART_STATE_READY) {
    g_uarts[id].evt_cb(id, UART_EVENT_TX_EMPTY);
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  uart_id_t id = uart_id_from_huart(huart);
  if (id >= BSP_UART_COUNT) {
    return;
  }

  if (Size == 0U) {
    uart_rx_try_start(id);
    return;
  }

  size_t const pushed = ringbuf_push(&g_rx_rb[id], g_rx_tmp[id], Size);

  /* Re-arm RX before notifying task (same reason as byte-at-a-time path). */
  uart_rx_try_start(id);

  if (g_uarts[id].evt_cb != NULL) {
    if (pushed > 0U) {
      g_uarts[id].evt_cb(id, UART_EVENT_RX_AVAILABLE);
    }
    if (pushed < Size) {
      g_uarts[id].evt_cb(id, UART_EVENT_RX_OVERFLOW);
    }
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);
  if (id >= BSP_UART_COUNT) {
    return;
  }

  if ((huart->ErrorCode & HAL_UART_ERROR_ORE) != 0U) {
    __HAL_UART_CLEAR_OREFLAG(huart);
  }
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  uart_rx_try_start(id);

  uart_tx_try_start(id);
}

BSP_UART_IRQ_HANDLERS
