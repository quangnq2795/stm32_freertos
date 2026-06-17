#include "ir_rx_drv.h"

#include "bsp_ir_rx_cfg.h"
#include "h_soft_timer.h"
#include "ir_rx_time.h"
#include "gpio.h"

#include "stm32_hal.h"

#include <string.h>

typedef struct
{
  uint16_t pulse_buf[BSP_IR_RX_MAX_PULSE_RING_CAP];
  volatile uint16_t pulse_count;
  volatile uint32_t last_edge_tick_us;
  ir_rx_event_fn_t event_fn;
  int idle_timer_id;
  int is_new_frame;
} ir_rx_channel_runtime_t;

static ir_rx_hw_channel_t s_hw_channels[BSP_IR_RX_COUNT] = BSP_IR_RX_DESCS;
static ir_rx_channel_runtime_t s_channel_rt[BSP_IR_RX_COUNT];

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

  if (s_channel_rt[channel].pulse_count == 0U) {
    return;
  }

  s_channel_rt[channel].is_new_frame = 1;

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
  uint16_t cap;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  cap = s_hw_channels[channel].hw.pulse_ring_cap;
  if (s_channel_rt[channel].pulse_count >= cap) {
    ir_rx_drv_idle_timer_disarm(channel);
    if (s_channel_rt[channel].event_fn != NULL) {
      s_channel_rt[channel].event_fn(channel, IR_RX_EVT_BUF_OVERFLOW);
    }
    return;
  }

  s_channel_rt[channel].pulse_buf[s_channel_rt[channel].pulse_count] = width_us;
  s_channel_rt[channel].pulse_count++;
}

void ir_rx_drv_on_gpio_edge(ir_rx_channel_id_t channel, uint16_t gpio_pin)
{
  uint32_t now_us;
  uint32_t delta_us;

  if (channel >= BSP_IR_RX_COUNT ||
      gpio_pin != s_hw_channels[channel].hw.gpio_pin) {
    return;
  }

  now_us = ir_rx_time_us();

  if (s_channel_rt[channel].is_new_frame != 0) {
    s_channel_rt[channel].is_new_frame = 0;
    s_channel_rt[channel].last_edge_tick_us = now_us;
    ir_rx_drv_idle_timer_arm(channel);
    return;
  }

  delta_us = now_us - s_channel_rt[channel].last_edge_tick_us;
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

  gpio_exti_irq_handler(s_hw_channels[channel].hw.gpio_pin);
}

static void ir_rx_drv_exti_cb(uint16_t pin, void *ctx)
{
  ir_rx_channel_id_t channel = (ir_rx_channel_id_t)(uintptr_t)ctx;

  ir_rx_drv_on_gpio_edge(channel, pin);
}

void ir_rx_drv_init(ir_rx_channel_id_t channel)
{
  const ir_rx_hw_desc_t *hw;

  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  hw = &s_hw_channels[channel].hw;

  s_channel_rt[channel].pulse_count = 0U;
  s_channel_rt[channel].last_edge_tick_us = 0U;
  s_channel_rt[channel].event_fn = NULL;
  s_channel_rt[channel].idle_timer_id = H_SOFT_TIMER_INVALID_ID;
  s_channel_rt[channel].is_new_frame = 1;
  ir_rx_drv_idle_timer_disarm(channel);

  if (channel == 0U) {
    ir_rx_time_init();
  }

  (void)gpio_config_input_exti(hw->gpio_port, hw->gpio_pin, GPIO_EXTI_EDGE_BOTH,
                               GPIO_PULLUP, hw->exti_irqn, hw->exti_irq_prio,
                               ir_rx_drv_exti_cb, (void *)(uintptr_t)channel);

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

  return (size_t)s_channel_rt[channel].pulse_count;
}

size_t ir_rx_drv_read_buffered_pulses(ir_rx_channel_id_t channel, uint16_t *out,
                                      size_t max_count)
{
  size_t available;
  size_t read_count;

  if (channel >= BSP_IR_RX_COUNT || out == NULL || max_count == 0U) {
    return 0U;
  }

  available = (size_t)s_channel_rt[channel].pulse_count;
  read_count = (available < max_count) ? available : max_count;
  if (read_count == 0U) {
    return 0U;
  }

  (void)memcpy(out, s_channel_rt[channel].pulse_buf,
               read_count * sizeof(uint16_t));
  s_channel_rt[channel].pulse_count = 0U;
  s_channel_rt[channel].is_new_frame = 1;



  return read_count;
}

void ir_rx_drv_flush_buffer(ir_rx_channel_id_t channel)
{
  if (channel >= BSP_IR_RX_COUNT) {
    return;
  }

  s_channel_rt[channel].pulse_count = 0U;
  s_channel_rt[channel].is_new_frame = 1;
  ir_rx_drv_idle_timer_disarm(channel);
}

BSP_IR_RX_IRQ_HANDLERS
