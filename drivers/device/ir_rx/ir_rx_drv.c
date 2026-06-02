#include "ir_rx_drv.h"

#include "bsp_ir_rx_cfg.h"
#include "h_soft_timer.h"
#include "ir_rx_time.h"
#include "ringbuf.h"

#include "stm32f2xx_hal.h"

typedef struct
{
  ringbuf_u8_t pulse_ring;
  volatile uint32_t last_edge_tick_us;
  ir_rx_event_fn_t event_fn;
  int idle_timer_id;
} ir_rx_channel_runtime_t;

static ir_rx_hw_channel_t s_hw_channels[BSP_IR_RX_COUNT] = BSP_IR_RX_DESCS;
static ir_rx_channel_runtime_t s_channel_rt[BSP_IR_RX_COUNT];
static uint8_t s_pulse_byte_storage[BSP_IR_RX_COUNT]
    [BSP_IR_RX_MAX_PULSE_RING_CAP * 2U];

static ir_rx_channel_id_t ir_rx_drv_channel_from_pin(uint16_t gpio_pin)
{
  for (ir_rx_channel_id_t ch = 0U; ch < BSP_IR_RX_COUNT; ++ch) {
    if (gpio_pin == s_hw_channels[ch].hw.gpio_pin) {
      return ch;
    }
  }
  return BSP_IR_RX_COUNT;
}

static void ir_rx_drv_idle_timer_disarm(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  if (s_channel_rt[channel].idle_timer_id >= 0) {
    h_soft_timer_unregister(s_channel_rt[channel].idle_timer_id);
    s_channel_rt[channel].idle_timer_id = H_SOFT_TIMER_INVALID_ID;
  }
}

static void ir_rx_drv_idle_timer_fired(void *ctx)
{
  ir_rx_channel_id_t channel = (ir_rx_channel_id_t)(uintptr_t)ctx;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  s_channel_rt[channel].idle_timer_id = H_SOFT_TIMER_INVALID_ID;

  if (ringbuf_len(&s_channel_rt[channel].pulse_ring) == 0U) {
    return;
  }

  if (s_channel_rt[channel].event_fn != NULL) {
    s_channel_rt[channel].event_fn(channel, IR_RX_EVT_BURST_READY);
  }
}

static void ir_rx_drv_idle_timer_arm(ir_rx_channel_id_t channel)
{
  int timer_id;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  ir_rx_drv_idle_timer_disarm(channel);

  timer_id = h_soft_timer_register(s_hw_channels[channel].hw.idle_timeout_us,
                                   ir_rx_drv_idle_timer_fired,
                                   (void *)(uintptr_t)channel);
  if (timer_id >= 0) {
    s_channel_rt[channel].idle_timer_id = timer_id;
  }
}

static void ir_rx_drv_push_pulse_width(ir_rx_channel_id_t channel, uint16_t width_us)
{
  uint8_t raw_le[2];

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  if (ringbuf_space(&s_channel_rt[channel].pulse_ring) < 2U) {
    ir_rx_drv_idle_timer_disarm(channel);
    if (s_channel_rt[channel].event_fn != NULL) {
      s_channel_rt[channel].event_fn(channel, IR_RX_EVT_BUF_OVERFLOW);
    }
    return;
  }

  raw_le[0] = (uint8_t)(width_us & 0xFFU);
  raw_le[1] = (uint8_t)(width_us >> 8);
  (void)ringbuf_push(&s_channel_rt[channel].pulse_ring, raw_le, 2U);
}

void ir_rx_drv_on_gpio_edge(ir_rx_channel_id_t channel, uint16_t gpio_pin)
{
  if (channel >= BSP_IR_RX_COUNT ||
      gpio_pin != s_hw_channels[channel].hw.gpio_pin) {
    return;
  }

  uint32_t now_us = ir_rx_time_us();
  uint32_t delta_us = now_us - s_channel_rt[channel].last_edge_tick_us;
  s_channel_rt[channel].last_edge_tick_us = now_us;

  if (delta_us > 0U && delta_us <= 0xFFFFU) {
    ir_rx_drv_push_pulse_width(channel, (uint16_t)delta_us);
  }

  ir_rx_drv_idle_timer_arm(channel);
}

void ir_rx_drv_exti_irq(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  HAL_GPIO_EXTI_IRQHandler(s_hw_channels[channel].hw.gpio_pin);
}

void ir_rx_drv_init(ir_rx_channel_id_t channel)
{
  const ir_rx_hw_desc_t *hw;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  hw = &s_hw_channels[channel].hw;

  s_channel_rt[channel].last_edge_tick_us = 0U;
  s_channel_rt[channel].event_fn = NULL;
  s_channel_rt[channel].idle_timer_id = H_SOFT_TIMER_INVALID_ID;
  ir_rx_drv_idle_timer_disarm(channel);

  ringbuf_init(&s_channel_rt[channel].pulse_ring, s_pulse_byte_storage[channel],
               (uint16_t)(hw->pulse_ring_cap * 2U));

  if (channel == 0U) {
    ir_rx_time_init();
  }

  if (hw->gpio_clk_enable != NULL) {
    hw->gpio_clk_enable();
  }

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = hw->gpio_pin;
  gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(hw->gpio_port, &gpio);

  HAL_NVIC_SetPriority(hw->exti_irqn, hw->exti_irq_prio, 0);
  HAL_NVIC_EnableIRQ(hw->exti_irqn);

  s_channel_rt[channel].last_edge_tick_us = ir_rx_time_us();
}

void ir_rx_drv_init_all(void)
{
  for (ir_rx_channel_id_t ch = 0U; ch < BSP_IR_RX_COUNT; ++ch) {
    ir_rx_drv_init(ch);
  }
}

void ir_rx_drv_set_event_fn(ir_rx_channel_id_t channel, ir_rx_event_fn_t fn)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  s_channel_rt[channel].event_fn = fn;
}

size_t ir_rx_drv_buffered_pulse_count(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return 0U;
  }

  return (size_t)(ringbuf_len(&s_channel_rt[channel].pulse_ring) / 2U);
}

size_t ir_rx_drv_read_buffered_pulses(ir_rx_channel_id_t channel, uint16_t *out,
                                      size_t max_count)
{
  size_t read_count = 0U;
  uint8_t raw_le[2];

  if (channel >= BSP_IR_RX_COUNT || out == NULL || max_count == 0U) {
    return 0U;
  }

  while (read_count < max_count &&
         ringbuf_pop(&s_channel_rt[channel].pulse_ring, raw_le, 2U) == 2U) {
    out[read_count] =
        (uint16_t)((uint16_t)raw_le[0] | ((uint16_t)raw_le[1] << 8));
    read_count++;
  }
  return read_count;
}

void ir_rx_drv_flush_buffer(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  ringbuf_reset(&s_channel_rt[channel].pulse_ring);
  ir_rx_drv_idle_timer_disarm(channel);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  ir_rx_channel_id_t channel = ir_rx_drv_channel_from_pin(GPIO_Pin);

  if (channel < BSP_IR_RX_COUNT) {
    ir_rx_drv_on_gpio_edge(channel, GPIO_Pin);
  }
}

BSP_IR_RX_IRQ_HANDLERS
