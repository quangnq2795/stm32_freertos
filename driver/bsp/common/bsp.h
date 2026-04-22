#pragma once

void bsp_init(void);
/** Called from FreeRTOS tick hook; forwards to HAL tick (see bsp.c). */
void bsp_sync_systick(void);
