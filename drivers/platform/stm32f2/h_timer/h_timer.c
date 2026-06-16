#include "h_timer.h"

#include "bsp_htimer_cfg.h"

#include "stm32_hal.h"

static TIM_HandleTypeDef s_htim;
static void (*s_expire_hook)(void);
static uint8_t s_oc_ready;

void h_timer_init(void)
{
  static uint8_t s_started;

  if (s_started != 0U) {
    return;
  }
  s_started = 1U;

  s_expire_hook = NULL;
  s_oc_ready = 0U;

  BSP_H_TIMER_CLK_ENABLE();

  s_htim.Instance = BSP_H_TIMER_INSTANCE;
  s_htim.Init.Prescaler = BSP_H_TIMER_PRESCALER;
  s_htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  s_htim.Init.Period = 0xFFFFFFFFU;
  s_htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  (void)HAL_TIM_Base_Init(&s_htim);
  (void)HAL_TIM_Base_Start(&s_htim);

  HAL_NVIC_SetPriority(BSP_H_TIMER_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(BSP_H_TIMER_IRQn);
}

uint32_t h_timer_tick_us(void)
{
  return __HAL_TIM_GET_COUNTER(&s_htim);
}

void h_timer_set_expire_hook(void (*hook)(void))
{
  s_expire_hook = hook;
}

static void h_timer_oc_ensure(void)
{
  TIM_OC_InitTypeDef oc = {0};

  if (s_oc_ready != 0U) {
    return;
  }

  oc.OCMode = TIM_OCMODE_TIMING;
  oc.Pulse = 0U;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  oc.OCFastMode = TIM_OCFAST_DISABLE;
  (void)HAL_TIM_OC_ConfigChannel(&s_htim, &oc, BSP_H_TIMER_CHANNEL);
  s_oc_ready = 1U;
}

void h_timer_schedule(uint32_t deadline_us)
{
  h_timer_oc_ensure();
  __HAL_TIM_CLEAR_FLAG(&s_htim, TIM_FLAG_CC1);
  __HAL_TIM_SET_COMPARE(&s_htim, BSP_H_TIMER_CHANNEL, deadline_us);
  (void)HAL_TIM_OC_Start_IT(&s_htim, BSP_H_TIMER_CHANNEL);
}

void h_timer_stop_compare(void)
{
  (void)HAL_TIM_OC_Stop_IT(&s_htim, BSP_H_TIMER_CHANNEL);
}

void h_timer_irq_handler(void)
{
  HAL_TIM_IRQHandler(&s_htim);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != BSP_H_TIMER_INSTANCE) {
    return;
  }

  /* HAL_TIM_IRQHandler clears CC1 before invoking this callback. */
  (void)HAL_TIM_OC_Stop_IT(htim, BSP_H_TIMER_CHANNEL);

  if (s_expire_hook != NULL) {
    s_expire_hook();
  }
}

BSP_H_TIMER_IRQ_HANDLERS
