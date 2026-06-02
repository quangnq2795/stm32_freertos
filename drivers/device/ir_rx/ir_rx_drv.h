#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bsp_ir_rx_cfg.h"
#include "stm32f2xx_hal.h"

typedef uint8_t ir_rx_channel_id_t;

typedef struct
{
  GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  IRQn_Type exti_irqn;
  uint8_t exti_irq_prio;
  void (*gpio_clk_enable)(void);
  uint32_t idle_timeout_us;
  uint16_t pulse_ring_cap;
} ir_rx_hw_desc_t;

typedef struct
{
  ir_rx_hw_desc_t hw;
} ir_rx_hw_channel_t;

typedef enum
{
  IR_RX_EVT_NONE = 0,
  IR_RX_EVT_BURST_READY,
  IR_RX_EVT_BUF_OVERFLOW,
} ir_rx_event_t;

typedef void (*ir_rx_event_fn_t)(ir_rx_channel_id_t channel, ir_rx_event_t event);

void ir_rx_drv_init(ir_rx_channel_id_t channel);
void ir_rx_drv_init_all(void);

void ir_rx_drv_set_event_fn(ir_rx_channel_id_t channel, ir_rx_event_fn_t fn);

size_t ir_rx_drv_buffered_pulse_count(ir_rx_channel_id_t channel);
size_t ir_rx_drv_read_buffered_pulses(ir_rx_channel_id_t channel, uint16_t *out,
                                      size_t max_count);
void ir_rx_drv_flush_buffer(ir_rx_channel_id_t channel);

void ir_rx_drv_exti_irq(ir_rx_channel_id_t channel);
void ir_rx_drv_on_gpio_edge(ir_rx_channel_id_t channel, uint16_t gpio_pin);
