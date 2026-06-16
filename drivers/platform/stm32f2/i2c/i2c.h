#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32_hal.h"

/*
 * Async I2C driver on HAL interrupt mode.
 * Completion callback may run from I2C or h_soft_timer ISR context.
 */

typedef uint8_t i2c_id_t;

typedef struct
{
  I2C_TypeDef *instance;
  uint32_t clock_speed;

  GPIO_TypeDef *scl_port;
  uint16_t scl_pin;
  uint8_t scl_af;

  GPIO_TypeDef *sda_port;
  uint16_t sda_pin;
  uint8_t sda_af;

  IRQn_Type ev_irqn;
  IRQn_Type er_irqn;

  uint32_t timeout_ms;

  I2C_HandleTypeDef hi2c;
} i2c_desc_t;

typedef enum
{
  I2C_DRV_OK = 0,
  I2C_DRV_ERR_PARAM = -1,
  I2C_DRV_ERR_IO = -2,
  I2C_DRV_ERR_BUSY = -3,
} i2c_drv_status_t;

typedef void (*i2c_done_cb_t)(i2c_id_t id, i2c_drv_status_t status, void *ctx);

void i2c_init(i2c_id_t id);

i2c_drv_status_t i2c_master_write_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                        const uint8_t *buf, size_t len,
                                        i2c_done_cb_t cb, void *ctx);
i2c_drv_status_t i2c_master_read_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                       uint8_t *buf, size_t len,
                                       i2c_done_cb_t cb, void *ctx);
i2c_drv_status_t i2c_mem_write_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                     uint16_t mem_addr, uint16_t mem_addr_size,
                                     const uint8_t *buf, size_t len,
                                     i2c_done_cb_t cb, void *ctx);
i2c_drv_status_t i2c_mem_read_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                      uint16_t mem_addr, uint16_t mem_addr_size,
                                      uint8_t *buf, size_t len,
                                      i2c_done_cb_t cb, void *ctx);
i2c_drv_status_t i2c_is_device_ready(i2c_id_t id, uint8_t dev_addr_7bit,
                                     uint32_t trials);

void i2c_ev_irq_handler(I2C_TypeDef *instance);
void i2c_er_irq_handler(I2C_TypeDef *instance);
