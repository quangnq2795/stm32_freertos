#pragma once

void bsp_init(void);
/** Called from FreeRTOS tick hook; forwards to HAL tick (see bsp.c). */
void sync_systick(void);
