#include "i2c.h"
#include "bsp_i2c_cfg.h"

#include "h_soft_timer.h"

static i2c_desc_t g_i2c[BSP_I2C_COUNT] = BSP_I2C_DESCS;
static uint8_t s_inited[BSP_I2C_COUNT];

typedef struct
{
  volatile uint8_t busy;
  volatile uint8_t aborting;
  i2c_done_cb_t cb;
  void *ctx;
  int timer_id;
  uint16_t active_dev_addr;
} i2c_runtime_t;

static i2c_runtime_t s_rt[BSP_I2C_COUNT];

static uint8_t i2c_id_valid(i2c_id_t id)
{
  return (id < BSP_I2C_COUNT) ? 1U : 0U;
}

static uint16_t i2c_hal_addr(uint8_t dev_addr_7bit)
{
  return (uint16_t)((uint16_t)dev_addr_7bit << 1);
}

static i2c_id_t i2c_id_from_hi2c(const I2C_HandleTypeDef *hi2c)
{
  for (i2c_id_t i = 0U; i < BSP_I2C_COUNT; ++i) {
    if (hi2c == &g_i2c[i].hi2c) {
      return i;
    }
  }
  return (i2c_id_t)BSP_I2C_COUNT;
}

static i2c_desc_t *i2c_desc_from_instance(I2C_TypeDef *instance)
{
  for (i2c_id_t i = 0U; i < BSP_I2C_COUNT; ++i) {
    if (g_i2c[i].hi2c.Instance == instance) {
      return &g_i2c[i];
    }
  }
  return NULL;
}

static void i2c_timer_stop(i2c_id_t id)
{
  if (s_rt[id].timer_id >= 0) {
    h_soft_timer_unregister(s_rt[id].timer_id);
    s_rt[id].timer_id = H_SOFT_TIMER_INVALID_ID;
  }
}

static void i2c_finish(i2c_id_t id, i2c_drv_status_t st)
{
  i2c_done_cb_t cb;
  void *ctx;

  if (s_rt[id].busy == 0U) {
    return;
  }

  i2c_timer_stop(id);
  s_rt[id].aborting = 0U;
  s_rt[id].busy = 0U;

  cb = s_rt[id].cb;
  ctx = s_rt[id].ctx;
  s_rt[id].cb = NULL;
  s_rt[id].ctx = NULL;

  if (cb != NULL) {
    cb(id, st, ctx);
  }
}

static void i2c_timeout_cb(void *arg)
{
  uintptr_t raw = (uintptr_t)arg;
  i2c_id_t id = (i2c_id_t)raw;

  if (!i2c_id_valid(id) || s_rt[id].busy == 0U) {
    return;
  }

  s_rt[id].timer_id = H_SOFT_TIMER_INVALID_ID;
  s_rt[id].aborting = 1U;
  (void)HAL_I2C_Master_Abort_IT(&g_i2c[id].hi2c, s_rt[id].active_dev_addr);
}

static void i2c_timeout_arm(i2c_id_t id, uint32_t timeout_ms)
{
  uint32_t timeout_us = timeout_ms * 1000U;

  if (timeout_us == 0U) {
    timeout_us = 1000U;
  }

  s_rt[id].timer_id =
      h_soft_timer_register(timeout_us, i2c_timeout_cb, (void *)(uintptr_t)id);
  if (s_rt[id].timer_id < 0) {
    s_rt[id].timer_id = H_SOFT_TIMER_INVALID_ID;
  }
}

static i2c_drv_status_t i2c_bus_acquire(i2c_id_t id)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  if (s_rt[id].busy != 0U) {
    if (primask == 0U) {
      __enable_irq();
    }
    return I2C_DRV_ERR_BUSY;
  }

  s_rt[id].busy = 1U;
  s_rt[id].aborting = 0U;
  s_rt[id].cb = NULL;
  s_rt[id].ctx = NULL;
  s_rt[id].timer_id = H_SOFT_TIMER_INVALID_ID;
  if (primask == 0U) {
    __enable_irq();
  }
  return I2C_DRV_OK;
}

static i2c_drv_status_t i2c_bus_try_busy(i2c_id_t id, i2c_done_cb_t cb,
                                         void *ctx)
{
  i2c_drv_status_t st;

  if (cb == NULL) {
    return I2C_DRV_ERR_PARAM;
  }

  st = i2c_bus_acquire(id);
  if (st != I2C_DRV_OK) {
    return st;
  }

  s_rt[id].cb = cb;
  s_rt[id].ctx = ctx;
  return I2C_DRV_OK;
}

static void i2c_bus_release(i2c_id_t id)
{
  i2c_timer_stop(id);
  s_rt[id].aborting = 0U;
  s_rt[id].busy = 0U;
  s_rt[id].cb = NULL;
  s_rt[id].ctx = NULL;
}

static i2c_drv_status_t i2c_async_fail(i2c_id_t id, i2c_done_cb_t cb, void *ctx)
{
  i2c_bus_release(id);
  if (cb != NULL) {
    cb(id, I2C_DRV_ERR_IO, ctx);
  }
  return I2C_DRV_ERR_IO;
}

static i2c_drv_status_t i2c_async_start(i2c_id_t id, uint32_t timeout_ms,
                                        i2c_done_cb_t cb, void *ctx,
                                        HAL_StatusTypeDef start_st)
{
  if (start_st != HAL_OK) {
    return i2c_async_fail(id, cb, ctx);
  }

  i2c_timeout_arm(id, timeout_ms);
  if (s_rt[id].timer_id < 0) {
    return i2c_async_fail(id, cb, ctx);
  }

  return I2C_DRV_OK;
}

static void i2c_gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af)
{
  GPIO_InitTypeDef gpio = {
    .Pin = pin,
    .Mode = GPIO_MODE_AF_OD,
    .Pull = GPIO_PULLUP,
    .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
    .Alternate = af,
  };

  HAL_GPIO_Init(port, &gpio);
}

static void i2c_gpio_init(const i2c_desc_t *d)
{
  if (d->gpio_clk_enable != NULL) {
    d->gpio_clk_enable();
  }

  i2c_gpio_init_af(d->scl_port, d->scl_pin, d->scl_af);
  i2c_gpio_init_af(d->sda_port, d->sda_pin, d->sda_af);
}

static void i2c_irq_enable(const i2c_desc_t *d, uint8_t enable)
{
  if ((int32_t)d->ev_irqn >= 0) {
    if (enable != 0U) {
      HAL_NVIC_EnableIRQ(d->ev_irqn);
    } else {
      HAL_NVIC_DisableIRQ(d->ev_irqn);
    }
  }

  if ((int32_t)d->er_irqn >= 0) {
    if (enable != 0U) {
      HAL_NVIC_EnableIRQ(d->er_irqn);
    } else {
      HAL_NVIC_DisableIRQ(d->er_irqn);
    }
  }
}

void i2c_ev_irq_handler(I2C_TypeDef *instance)
{
  i2c_desc_t *d = i2c_desc_from_instance(instance);

  if (d != NULL) {
    HAL_I2C_EV_IRQHandler(&d->hi2c);
  }
}

void i2c_er_irq_handler(I2C_TypeDef *instance)
{
  i2c_desc_t *d = i2c_desc_from_instance(instance);

  if (d != NULL) {
    HAL_I2C_ER_IRQHandler(&d->hi2c);
  }
}

void i2c_init(i2c_id_t id)
{
  i2c_desc_t *d;
  I2C_HandleTypeDef *hi2c;

  if (!i2c_id_valid(id) || s_inited[id] != 0U) {
    return;
  }

  d = &g_i2c[id];
  hi2c = &d->hi2c;

  s_rt[id].busy = 0U;
  s_rt[id].aborting = 0U;
  s_rt[id].cb = NULL;
  s_rt[id].ctx = NULL;
  s_rt[id].timer_id = H_SOFT_TIMER_INVALID_ID;

  i2c_gpio_init(d);

  if (d->i2c_clk_enable != NULL) {
    d->i2c_clk_enable();
  }

  if ((int32_t)d->ev_irqn >= 0) {
    HAL_NVIC_SetPriority(d->ev_irqn, BSP_I2C_IRQ_PRIO, 0);
  }
  if ((int32_t)d->er_irqn >= 0) {
    HAL_NVIC_SetPriority(d->er_irqn, BSP_I2C_IRQ_PRIO, 0);
  }
  i2c_irq_enable(d, 1U);

  hi2c->Instance = d->instance;
  hi2c->Init.ClockSpeed = d->clock_speed;
  hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c->Init.OwnAddress1 = 0U;
  hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c->Init.OwnAddress2 = 0U;
  hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  (void)HAL_I2C_Init(hi2c);
  s_inited[id] = 1U;
}

i2c_drv_status_t i2c_master_write_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                        const uint8_t *buf, size_t len,
                                        i2c_done_cb_t cb, void *ctx)
{
  i2c_desc_t *d;
  i2c_drv_status_t lock_st;
  HAL_StatusTypeDef st;

  if (!i2c_id_valid(id) || s_inited[id] == 0U || buf == NULL || len == 0U ||
      len > 0xFFFFU) {
    return I2C_DRV_ERR_PARAM;
  }

  lock_st = i2c_bus_try_busy(id, cb, ctx);
  if (lock_st != I2C_DRV_OK) {
    return lock_st;
  }

  d = &g_i2c[id];
  s_rt[id].active_dev_addr = i2c_hal_addr(dev_addr_7bit);
  st = HAL_I2C_Master_Transmit_IT(&d->hi2c, s_rt[id].active_dev_addr,
                                  (uint8_t *)buf, (uint16_t)len);
  return i2c_async_start(id, d->timeout_ms, cb, ctx, st);
}

i2c_drv_status_t i2c_master_read_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                       uint8_t *buf, size_t len,
                                       i2c_done_cb_t cb, void *ctx)
{
  i2c_desc_t *d;
  i2c_drv_status_t lock_st;
  HAL_StatusTypeDef st;

  if (!i2c_id_valid(id) || s_inited[id] == 0U || buf == NULL || len == 0U ||
      len > 0xFFFFU) {
    return I2C_DRV_ERR_PARAM;
  }

  lock_st = i2c_bus_try_busy(id, cb, ctx);
  if (lock_st != I2C_DRV_OK) {
    return lock_st;
  }

  d = &g_i2c[id];
  s_rt[id].active_dev_addr = i2c_hal_addr(dev_addr_7bit);
  st = HAL_I2C_Master_Receive_IT(&d->hi2c, s_rt[id].active_dev_addr, buf,
                                 (uint16_t)len);
  return i2c_async_start(id, d->timeout_ms, cb, ctx, st);
}

i2c_drv_status_t i2c_mem_write_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                     uint16_t mem_addr, uint16_t mem_addr_size,
                                     const uint8_t *buf, size_t len,
                                     i2c_done_cb_t cb, void *ctx)
{
  i2c_desc_t *d;
  i2c_drv_status_t lock_st;
  HAL_StatusTypeDef st;

  if (!i2c_id_valid(id) || s_inited[id] == 0U || buf == NULL || len == 0U ||
      len > 0xFFFFU ||
      (mem_addr_size != I2C_MEMADD_SIZE_8BIT &&
       mem_addr_size != I2C_MEMADD_SIZE_16BIT)) {
    return I2C_DRV_ERR_PARAM;
  }

  lock_st = i2c_bus_try_busy(id, cb, ctx);
  if (lock_st != I2C_DRV_OK) {
    return lock_st;
  }

  d = &g_i2c[id];
  s_rt[id].active_dev_addr = i2c_hal_addr(dev_addr_7bit);
  st = HAL_I2C_Mem_Write_IT(&d->hi2c, s_rt[id].active_dev_addr, mem_addr,
                            mem_addr_size, (uint8_t *)buf, (uint16_t)len);
  return i2c_async_start(id, d->timeout_ms, cb, ctx, st);
}

i2c_drv_status_t i2c_mem_read_async(i2c_id_t id, uint8_t dev_addr_7bit,
                                    uint16_t mem_addr, uint16_t mem_addr_size,
                                    uint8_t *buf, size_t len,
                                    i2c_done_cb_t cb, void *ctx)
{
  i2c_desc_t *d;
  i2c_drv_status_t lock_st;
  HAL_StatusTypeDef st;

  if (!i2c_id_valid(id) || s_inited[id] == 0U || buf == NULL || len == 0U ||
      len > 0xFFFFU ||
      (mem_addr_size != I2C_MEMADD_SIZE_8BIT &&
       mem_addr_size != I2C_MEMADD_SIZE_16BIT)) {
    return I2C_DRV_ERR_PARAM;
  }

  lock_st = i2c_bus_try_busy(id, cb, ctx);
  if (lock_st != I2C_DRV_OK) {
    return lock_st;
  }

  d = &g_i2c[id];
  s_rt[id].active_dev_addr = i2c_hal_addr(dev_addr_7bit);
  st = HAL_I2C_Mem_Read_IT(&d->hi2c, s_rt[id].active_dev_addr, mem_addr,
                           mem_addr_size, buf, (uint16_t)len);
  return i2c_async_start(id, d->timeout_ms, cb, ctx, st);
}

i2c_drv_status_t i2c_is_device_ready(i2c_id_t id, uint8_t dev_addr_7bit,
                                     uint32_t trials)
{
  i2c_desc_t *d;
  HAL_StatusTypeDef st;
  i2c_drv_status_t lock_st;

  if (!i2c_id_valid(id) || s_inited[id] == 0U || trials == 0U) {
    return I2C_DRV_ERR_PARAM;
  }

  lock_st = i2c_bus_acquire(id);
  if (lock_st != I2C_DRV_OK) {
    return lock_st;
  }

  d = &g_i2c[id];
  st = HAL_I2C_IsDeviceReady(&d->hi2c, i2c_hal_addr(dev_addr_7bit), trials,
                             d->timeout_ms);
  i2c_bus_release(id);
  return (st == HAL_OK) ? I2C_DRV_OK : I2C_DRV_ERR_IO;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (i2c_id_valid(id)) {
    i2c_finish(id, I2C_DRV_OK);
  }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (i2c_id_valid(id)) {
    i2c_finish(id, I2C_DRV_OK);
  }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (i2c_id_valid(id)) {
    i2c_finish(id, I2C_DRV_OK);
  }
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (i2c_id_valid(id)) {
    i2c_finish(id, I2C_DRV_OK);
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (i2c_id_valid(id)) {
    i2c_finish(id, I2C_DRV_ERR_IO);
  }
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
  i2c_id_t id = i2c_id_from_hi2c(hi2c);

  if (!i2c_id_valid(id) || s_rt[id].busy == 0U) {
    return;
  }

  if (s_rt[id].aborting != 0U) {
    i2c_finish(id, I2C_DRV_ERR_IO);
  } else {
    i2c_finish(id, I2C_DRV_OK);
  }
}

BSP_I2C_IRQ_HANDLERS
