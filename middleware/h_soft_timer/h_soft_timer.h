#pragma once

#include <stdint.h>

typedef void (*h_soft_timer_callback_t)(void *ctx);

#define H_SOFT_TIMER_INVALID_ID  (-1)

int h_soft_timer_register(uint32_t delay_us, h_soft_timer_callback_t cb, void *ctx);
void h_soft_timer_unregister(int id);

uint32_t h_soft_timer_tick(void);

void h_soft_timer_init(void);
