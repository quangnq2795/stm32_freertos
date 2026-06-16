#pragma once

#include "stm32_hal.h"

#define BSP_H_TIMER_INSTANCE      TIM2
#define BSP_H_TIMER_CLK_ENABLE()  __HAL_RCC_TIM2_CLK_ENABLE()
#define BSP_H_TIMER_IRQn          TIM2_IRQn
#define BSP_H_TIMER_PRESCALER     59U
#define BSP_H_TIMER_CHANNEL       TIM_CHANNEL_1

#ifndef BSP_H_TIMER_IRQ_HANDLER
#define BSP_H_TIMER_IRQ_HANDLER(TIMn) \
  void TIMn##_IRQHandler(void) { h_timer_irq_handler(); }
#endif

#define BSP_H_TIMER_IRQ_HANDLERS \
  BSP_H_TIMER_IRQ_HANDLER(TIM2)
