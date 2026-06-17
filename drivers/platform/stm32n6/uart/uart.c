#include "uart.h"
#include "bsp_uart_cfg.h"
#include "clk.h"

#include "ringbuf.h"

static int uart_hw_init(const uart_desc_t *d, UART_HandleTypeDef *hu);

static uart_desc_t g_uarts[BSP_UART_COUNT] = BSP_UART_DESCS;
static ringbuf_u8_t g_rx_rb[BSP_UART_COUNT];
static uint8_t g_rx_storage[BSP_UART_COUNT][UART_RX_RING_SIZE];
static uint8_t g_rx_tmp[BSP_UART_COUNT][UART_RX_IT_CHUNK];

static ringbuf_u8_t g_tx_rb[BSP_UART_COUNT];
static uint8_t g_tx_storage[BSP_UART_COUNT][UART_TX_RING_SIZE];
static uint8_t g_tx_tmp[BSP_UART_COUNT][UART_TX_IT_CHUNK];
static volatile uint8_t s_tx_active[BSP_UART_COUNT];

static uint8_t s_inited[BSP_UART_COUNT];

static uart_id_t uart_id_from_huart(const UART_HandleTypeDef *huart)
{
  for (uart_id_t i = 0U; i < BSP_UART_COUNT; ++i) {
    if (huart == &g_uarts[i].huart) {
      return i;
    }
  }
  return (uart_id_t)BSP_UART_COUNT;
}

static uint8_t uart_id_valid(uart_id_t id)
{
  return (id < BSP_UART_COUNT) ? 1U : 0U;
}

static void uart_notify(uart_id_t id, uart_event_t evt)
{
  if (uart_id_valid(id) && g_uarts[id].evt_cb != NULL) {
    g_uarts[id].evt_cb(id, evt);
  }
}

static void uart_rx_kick(uart_id_t id)
{
  (void)HAL_UARTEx_ReceiveToIdle_IT(&g_uarts[id].huart,
                                    g_rx_tmp[id],
                                    UART_RX_IT_CHUNK);
}

/*
 * Pop the next chunk from the TX ring and start an interrupt transfer.
 * Sole consumer of g_tx_rb: runs from uart_write under a critical section
 * (only when idle) or from the TxCplt ISR (while active) — never both at once.
 * When the ring is empty it clears s_tx_active and emits TX_EMPTY.
 */
static void uart_tx_kick(uart_id_t id)
{
  size_t n = ringbuf_pop(&g_tx_rb[id], g_tx_tmp[id], UART_TX_IT_CHUNK);

  if (n == 0U) {
    s_tx_active[id] = 0U;
    uart_notify(id, UART_EVENT_TX_EMPTY);
    return;
  }

  if (HAL_UART_Transmit_IT(&g_uarts[id].huart, g_tx_tmp[id], (uint16_t)n)
      != HAL_OK) {
    s_tx_active[id] = 0U;
  }
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
  uart_desc_t *d;
  UART_HandleTypeDef *hu;

  if (!uart_id_valid(id) || s_inited[id] != 0U) {
    return;
  }

  d = &g_uarts[id];
  hu = &d->huart;

  if ((int32_t)d->irqn >= 0) {
    HAL_NVIC_SetPriority(d->irqn, 6, 0);
    HAL_NVIC_EnableIRQ(d->irqn);
  }

  ringbuf_init(&g_rx_rb[id], g_rx_storage[id], UART_RX_RING_SIZE);
  ringbuf_init(&g_tx_rb[id], g_tx_storage[id], UART_TX_RING_SIZE);
  s_tx_active[id] = 0U;

  if (uart_hw_init(d, hu) != 0) {
    return;
  }

  uart_rx_kick(id);
  s_inited[id] = 1U;
}

void uart_register_event(uart_id_t id, uart_event_callback_t cb)
{
  if (!uart_id_valid(id)) {
    return;
  }
  g_uarts[id].evt_cb = cb;
}

size_t uart_read(uart_id_t id, uint8_t *out, size_t max_len)
{
  if (!uart_id_valid(id) || out == NULL || max_len == 0U) {
    return 0U;
  }

  return ringbuf_pop(&g_rx_rb[id], out, max_len);
}

size_t uart_write(uart_id_t id, const uint8_t *buf, size_t len)
{
  size_t pushed;
  uint32_t primask;

  if (!uart_id_valid(id) || s_inited[id] == 0U || buf == NULL || len == 0U) {
    return 0U;
  }

  pushed = ringbuf_push(&g_tx_rb[id], buf, len);
  if (pushed == 0U) {
    return 0U;
  }

  /* Start draining only when idle. The check-and-set excludes the TxCplt ISR
   * so the ring keeps a single active consumer. */
  primask = __get_PRIMASK();
  __disable_irq();
  if (s_tx_active[id] == 0U) {
    s_tx_active[id] = 1U;
    if (primask == 0U) {
      __enable_irq();
    }
    uart_tx_kick(id);
  } else if (primask == 0U) {
    __enable_irq();
  }

  return pushed;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);

  if (!uart_id_valid(id)) {
    return;
  }

  /* Continue draining the ring; emits TX_EMPTY once it is empty. */
  uart_tx_kick(id);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  uart_id_t id = uart_id_from_huart(huart);

  if (!uart_id_valid(id)) {
    return;
  }

  if (Size == 0U) {
    uart_rx_kick(id);
    return;
  }

  size_t pushed = ringbuf_push(&g_rx_rb[id], g_rx_tmp[id], Size);

  uart_rx_kick(id);

  if (pushed > 0U) {
    uart_notify(id, UART_EVENT_RX_AVAILABLE);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);

  if (!uart_id_valid(id)) {
    return;
  }

  if ((huart->ErrorCode & HAL_UART_ERROR_ORE) != 0U) {
    __HAL_UART_CLEAR_OREFLAG(huart);
  }
  huart->ErrorCode = HAL_UART_ERROR_NONE;

  uart_rx_kick(id);

  /* If an error left the transmitter idle with bytes still queued, resume. */
  if (s_tx_active[id] != 0U && huart->gState == HAL_UART_STATE_READY) {
    uart_tx_kick(id);
  }
}

BSP_UART_IRQ_HANDLERS

/* ---- MCU-specific (STM32N6) ---- */

static void uart_gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af)
{
  GPIO_InitTypeDef gpio = {
    .Pin = pin,
    .Mode = GPIO_MODE_AF_PP,
    .Pull = GPIO_NOPULL,
    .Speed = GPIO_SPEED_FREQ_LOW,
    .Alternate = af,
  };

  HAL_GPIO_Init(port, &gpio);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);
  const uart_desc_t *d;

  if (!uart_id_valid(id)) {
    return;
  }
  d = &g_uarts[id];

  (void)clk_config_uart_source(d->instance);
  clk_enable_gpio_port(d->tx_port);
  clk_enable_gpio_port(d->rx_port);
  clk_enable_uart(d->instance);

  uart_gpio_init_af(d->tx_port, d->tx_pin, d->tx_af);
  uart_gpio_init_af(d->rx_port, d->rx_pin, d->rx_af);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);
  const uart_desc_t *d;

  if (!uart_id_valid(id)) {
    return;
  }
  d = &g_uarts[id];

  HAL_GPIO_DeInit(d->tx_port, d->tx_pin);
  HAL_GPIO_DeInit(d->rx_port, d->rx_pin);
  clk_disable_uart(d->instance);
}

static int uart_hw_init(const uart_desc_t *d, UART_HandleTypeDef *hu)
{
  hu->Instance = d->instance;
  hu->Init.BaudRate = d->baudrate;
  hu->Init.WordLength = UART_WORDLENGTH_8B;
  hu->Init.StopBits = UART_STOPBITS_1;
  hu->Init.Parity = d->parity;
  hu->Init.Mode = UART_MODE_TX_RX;
  hu->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hu->Init.OverSampling = UART_OVERSAMPLING_16;
  hu->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hu->Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hu->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  return (HAL_UART_Init(hu) == HAL_OK) ? 0 : -1;
}
