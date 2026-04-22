#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"

void task_hmi_create(void);
void task_cli_create(void);

void vApplicationTickHook(void)
{
  bsp_sync_systick();
}

void os_init(void)
{
  task_hmi_create();
  task_cli_create();
}

void os_start(void)
{
  vTaskStartScheduler();
}
