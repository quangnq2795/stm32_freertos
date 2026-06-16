#pragma once

#include <stdint.h>

void h_timer_init(void);
uint32_t h_timer_tick_us(void);

void h_timer_schedule(uint32_t deadline_us);
void h_timer_stop_compare(void);
void h_timer_set_expire_hook(void (*hook)(void));
