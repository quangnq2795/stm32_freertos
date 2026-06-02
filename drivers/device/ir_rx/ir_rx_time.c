#include "ir_rx_time.h"

#include "h_timer.h"

void ir_rx_time_init(void)
{
  /* TIM2 started by h_soft_timer_init() in os_init. */
}

uint32_t ir_rx_time_us(void)
{
  return h_timer_tick_us();
}
