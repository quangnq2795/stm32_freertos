#include "FreeRTOS.h"
#include "task.h"

#include "taskmanager.h"

void task_cli_create(void);

void os_init(void)
{
  tm_system_init();
  task_cli_create();
}

void os_start(void)
{
  vTaskStartScheduler();
}
