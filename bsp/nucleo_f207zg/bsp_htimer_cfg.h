#pragma once

#include "stm32_hal.h"

/* TIM2: 1 us free-running counter + one compare channel for middleware h_timer. */

#define BSP_H_TIMER_INSTANCE      TIM2
#define BSP_H_TIMER_CLK_ENABLE()  __HAL_RCC_TIM2_CLK_ENABLE()
#define BSP_H_TIMER_IRQn          TIM2_IRQn
#define BSP_H_TIMER_CHANNEL       TIM_CHANNEL_1
#define BSP_H_TIMER_CC_FLAG       TIM_FLAG_CC1   /* phải khớp BSP_H_TIMER_CHANNEL */

/*
 * Bộ đếm chạy ở 1 MHz (1 tick = 1 us). Prescaler được tính động trong
 * h_timer_init từ clock thực tế của timer thay vì hằng số, để bám theo cấu hình
 * clock (tránh sai khi đổi SYSCLK).
 *
 * TIM2 nằm trên APB1; khi APB1 prescaler > 1 (ở đây /4) thì clock timer = 2×PCLK1.
 * Với PCLK1 30 MHz → timer 60 MHz.
 */
#define BSP_H_TIMER_TICK_HZ       1000000UL
#define BSP_H_TIMER_KERNEL_HZ     (2UL * HAL_RCC_GetPCLK1Freq())

#ifndef BSP_H_TIMER_IRQ_HANDLER
#define BSP_H_TIMER_IRQ_HANDLER(TIMn) \
  void TIMn##_IRQHandler(void) { h_timer_irq_handler(); }
#endif

#define BSP_H_TIMER_IRQ_HANDLERS \
  BSP_H_TIMER_IRQ_HANDLER(TIM2)
