#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bsp_ir_tx_cfg.h"
#include "stm32_hal.h"

typedef uint8_t ir_tx_channel_id_t;

typedef struct
{
  GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  uint8_t gpio_af;
  TIM_TypeDef *tim_pwm;
  uint32_t tim_channel;
  uint16_t tim_prescaler;
  uint16_t tim_period;
  uint16_t tim_pulse;
} ir_tx_hw_desc_t;

typedef struct
{
  ir_tx_hw_desc_t hw;
} ir_tx_hw_channel_t;

typedef struct
{
  uint16_t mark_us;
  uint16_t space_us;
} ir_tx_wave_t;

typedef enum
{
  IR_TX_XMIT_OK = 0,
  IR_TX_XMIT_BUSY,
} ir_tx_xmit_status_t;

typedef void (*ir_tx_complete_fn_t)(ir_tx_channel_id_t channel,
                                    ir_tx_xmit_status_t status, void *ctx);

void ir_tx_drv_init(ir_tx_channel_id_t channel);
void ir_tx_drv_init_all(void);

void ir_tx_drv_set_complete_fn(ir_tx_channel_id_t channel, ir_tx_complete_fn_t fn,
                               void *ctx);

int ir_tx_drv_start_waves(ir_tx_channel_id_t channel, const ir_tx_wave_t *waves,
                          size_t wave_count);

uint8_t ir_tx_drv_is_busy(ir_tx_channel_id_t channel);
