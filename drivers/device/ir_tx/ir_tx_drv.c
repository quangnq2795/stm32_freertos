#include "ir_tx_drv.h"

#include "bsp_ir_tx_cfg.h"
#include "h_soft_timer.h"

#include "stm32f2xx_hal.h"

#define IR_TX_DRV_MAX_SEGMENTS  80U

typedef struct
{
  uint16_t duration_us;
  uint8_t carrier_on;
} ir_tx_segment_t;

typedef struct
{
  TIM_HandleTypeDef tim_handle;
  ir_tx_segment_t segment_buf[IR_TX_DRV_MAX_SEGMENTS];
  volatile size_t segment_total;
  volatile size_t segment_index;
  volatile uint8_t is_transmitting;
  int segment_timer_id;
  ir_tx_complete_fn_t complete_fn;
  void *complete_ctx;
} ir_tx_channel_runtime_t;

static ir_tx_hw_channel_t s_hw_channels[BSP_IR_TX_COUNT] = BSP_IR_TX_DESCS;
static ir_tx_channel_runtime_t s_channel_rt[BSP_IR_TX_COUNT];

static void ir_tx_drv_notify_complete(ir_tx_channel_id_t channel,
                                      ir_tx_xmit_status_t status);
static void ir_tx_drv_finish(ir_tx_channel_id_t channel);
static void ir_tx_drv_segment_timer_cb(void *ctx);
static void ir_tx_drv_play_next_segment(ir_tx_channel_id_t channel);
static size_t ir_tx_drv_flatten_waves(ir_tx_channel_id_t channel,
                                      const ir_tx_wave_t *waves,
                                      size_t wave_count);

static void ir_tx_drv_carrier_on(ir_tx_channel_id_t channel)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  (void)HAL_TIM_PWM_Start(&s_channel_rt[channel].tim_handle,
                          s_hw_channels[channel].hw.tim_channel);
}

static void ir_tx_drv_carrier_off(ir_tx_channel_id_t channel)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  (void)HAL_TIM_PWM_Stop(&s_channel_rt[channel].tim_handle,
                         s_hw_channels[channel].hw.tim_channel);
}

static void ir_tx_drv_notify_complete(ir_tx_channel_id_t channel,
                                      ir_tx_xmit_status_t status)
{
  ir_tx_complete_fn_t fn;
  void *ctx;

  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  fn = s_channel_rt[channel].complete_fn;
  ctx = s_channel_rt[channel].complete_ctx;

  if (fn != NULL) {
    fn(channel, status, ctx);
  }
}

static void ir_tx_drv_finish(ir_tx_channel_id_t channel)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  if (s_channel_rt[channel].segment_timer_id != H_SOFT_TIMER_INVALID_ID) {
    h_soft_timer_unregister(s_channel_rt[channel].segment_timer_id);
    s_channel_rt[channel].segment_timer_id = H_SOFT_TIMER_INVALID_ID;
  }
  ir_tx_drv_carrier_off(channel);
  s_channel_rt[channel].is_transmitting = 0U;
  ir_tx_drv_notify_complete(channel, IR_TX_XMIT_OK);
}

static void ir_tx_drv_play_next_segment(ir_tx_channel_id_t channel)
{
  int timer_id;

  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  if (s_channel_rt[channel].segment_index >= s_channel_rt[channel].segment_total) {
    ir_tx_drv_finish(channel);
    return;
  }

  const ir_tx_segment_t *seg =
      &s_channel_rt[channel].segment_buf[s_channel_rt[channel].segment_index];
  if (seg->carrier_on != 0U) {
    ir_tx_drv_carrier_on(channel);
  } else {
    ir_tx_drv_carrier_off(channel);
  }

  s_channel_rt[channel].segment_index++;

  timer_id = h_soft_timer_register((uint32_t)seg->duration_us,
                                   ir_tx_drv_segment_timer_cb,
                                   (void *)(uintptr_t)channel);
  if (timer_id == H_SOFT_TIMER_INVALID_ID) {
    ir_tx_drv_finish(channel);
    return;
  }
  s_channel_rt[channel].segment_timer_id = timer_id;
}

static void ir_tx_drv_segment_timer_cb(void *ctx)
{
  ir_tx_channel_id_t channel = (ir_tx_channel_id_t)(uintptr_t)ctx;

  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }
  if (s_channel_rt[channel].is_transmitting == 0U) {
    return;
  }

  s_channel_rt[channel].segment_timer_id = H_SOFT_TIMER_INVALID_ID;
  ir_tx_drv_play_next_segment(channel);
}

static size_t ir_tx_drv_flatten_waves(ir_tx_channel_id_t channel,
                                      const ir_tx_wave_t *waves,
                                      size_t wave_count)
{
  size_t segment_count = 0U;

  if (channel >= BSP_IR_TX_COUNT || waves == NULL) {
    return 0U;
  }

  for (size_t wave_idx = 0U; wave_idx < wave_count &&
                             segment_count < IR_TX_DRV_MAX_SEGMENTS;
       wave_idx++) {
    if (waves[wave_idx].mark_us != 0U) {
      s_channel_rt[channel].segment_buf[segment_count].duration_us =
          waves[wave_idx].mark_us;
      s_channel_rt[channel].segment_buf[segment_count].carrier_on = 1U;
      segment_count++;
    }
    if (waves[wave_idx].space_us != 0U && segment_count < IR_TX_DRV_MAX_SEGMENTS) {
      s_channel_rt[channel].segment_buf[segment_count].duration_us =
          waves[wave_idx].space_us;
      s_channel_rt[channel].segment_buf[segment_count].carrier_on = 0U;
      segment_count++;
    }
  }
  return segment_count;
}

void ir_tx_drv_init(ir_tx_channel_id_t channel)
{
  const ir_tx_hw_desc_t *hw;

  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  hw = &s_hw_channels[channel].hw;

  if (hw->tim_clk_enable != NULL) {
    hw->tim_clk_enable();
  }
  if (hw->gpio_clk_enable != NULL) {
    hw->gpio_clk_enable();
  }

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin = hw->gpio_pin;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Alternate = hw->gpio_af;
  HAL_GPIO_Init(hw->gpio_port, &gpio);

  s_channel_rt[channel].tim_handle.Instance = hw->tim_pwm;
  s_channel_rt[channel].tim_handle.Init.Prescaler = hw->tim_prescaler;
  s_channel_rt[channel].tim_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  s_channel_rt[channel].tim_handle.Init.Period = hw->tim_period;
  s_channel_rt[channel].tim_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_channel_rt[channel].tim_handle.Init.RepetitionCounter = 0;
  s_channel_rt[channel].tim_handle.Init.AutoReloadPreload =
      TIM_AUTORELOAD_PRELOAD_DISABLE;

  (void)HAL_TIM_PWM_Init(&s_channel_rt[channel].tim_handle);

  TIM_OC_InitTypeDef oc = {0};
  oc.OCMode = TIM_OCMODE_PWM1;
  oc.Pulse = hw->tim_pulse;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  oc.OCFastMode = TIM_OCFAST_DISABLE;
  (void)HAL_TIM_PWM_ConfigChannel(&s_channel_rt[channel].tim_handle, &oc,
                                  hw->tim_channel);

  ir_tx_drv_carrier_off(channel);
  s_channel_rt[channel].is_transmitting = 0U;
  s_channel_rt[channel].complete_fn = NULL;
  s_channel_rt[channel].complete_ctx = NULL;
  s_channel_rt[channel].segment_timer_id = H_SOFT_TIMER_INVALID_ID;
}

void ir_tx_drv_init_all(void)
{
  for (ir_tx_channel_id_t ch = 0U; ch < BSP_IR_TX_COUNT; ++ch) {
    ir_tx_drv_init(ch);
  }
}

void ir_tx_drv_set_complete_fn(ir_tx_channel_id_t channel, ir_tx_complete_fn_t fn,
                               void *ctx)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return;
  }

  s_channel_rt[channel].complete_fn = fn;
  s_channel_rt[channel].complete_ctx = ctx;
}

uint8_t ir_tx_drv_is_busy(ir_tx_channel_id_t channel)
{
  if (channel >= BSP_IR_TX_COUNT) {
    return 0U;
  }

  return s_channel_rt[channel].is_transmitting;
}

int ir_tx_drv_start_waves(ir_tx_channel_id_t channel, const ir_tx_wave_t *waves,
                          size_t wave_count)
{
  if (channel >= BSP_IR_TX_COUNT || waves == NULL || wave_count == 0U) {
    return -1;
  }
  if (s_channel_rt[channel].is_transmitting != 0U) {
    return -1;
  }

  s_channel_rt[channel].segment_total =
      ir_tx_drv_flatten_waves(channel, waves, wave_count);
  if (s_channel_rt[channel].segment_total == 0U) {
    return -1;
  }

  s_channel_rt[channel].segment_index = 0U;
  s_channel_rt[channel].is_transmitting = 1U;
  ir_tx_drv_play_next_segment(channel);
  return 0;
}
