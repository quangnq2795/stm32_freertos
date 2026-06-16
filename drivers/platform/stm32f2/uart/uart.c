#include "uart.h"
#include "bsp_uart_cfg.h"
#include "clk.h"

#include "ringbuf.h"

static int uart_hw_init(const uart_desc_t *d, UART_HandleTypeDef *hu);

static uart_desc_t g_uarts[BSP_UART_COUNT] = BSP_UART_DESCS;
static ringbuf_u8_t g_rx_rb[BSP_UART_COUNT];
static uint8_t g_rx_storage[BSP_UART_COUNT][UART_RX_RING_SIZE];
static uint8_t g_rx_tmp[BSP_UART_COUNT][UART_RX_IT_CHUNK];

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
  UART_HandleTypeDef *hu;

  if (!uart_id_valid(id) || buf == NULL || len == 0U) {
    return 0U;
  }

  hu = &g_uarts[id].huart;
  if (hu->gState != HAL_UART_STATE_READY) {
    return 0U;
  }

  if (len > (size_t)UINT16_MAX) {
    len = (size_t)UINT16_MAX;
  }

  if (HAL_UART_Transmit_IT(hu, (uint8_t *)buf, (uint16_t)len) != HAL_OK) {
    return 0U;
  }

  return len;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  uart_id_t id = uart_id_from_huart(huart);

  if (!uart_id_valid(id)) {
    return;
  }

  uart_notify(id, UART_EVENT_TX_EMPTY);
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

  if (huart->gState == HAL_UART_STATE_READY) {
    uart_notify(id, UART_EVENT_TX_EMPTY);
  }
}

BSP_UART_IRQ_HANDLERS

/* ---- MCU-specific (STM32F2) ---- */

static void uart_gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af,
                              uint32_t pull)
{
  GPIO_InitTypeDef gpio = {
    .Pin = pin,
    .Mode = GPIO_MODE_AF_PP,
    .Pull = pull,
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

  clk_enable_gpio_port(d->tx_port);
  clk_enable_gpio_port(d->rx_port);
  (void)clk_config_uart_source(d->instance);
  clk_enable_uart(d->instance);

  uart_gpio_init_af(d->tx_port, d->tx_pin, d->tx_af, GPIO_NOPULL);
  uart_gpio_init_af(d->rx_port, d->rx_pin, d->rx_af, GPIO_PULLUP);
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

  return (HAL_UART_Init(hu) == HAL_OK) ? 0 : -1;
}
