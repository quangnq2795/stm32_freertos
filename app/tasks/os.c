#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"

void task_cli_create(void);

void vApplicationTickHook(void)
{
  sync_systick();
}

void os_init(void)
{
  task_cli_create();
}

void os_start(void)
{
  vTaskStartScheduler();
}
