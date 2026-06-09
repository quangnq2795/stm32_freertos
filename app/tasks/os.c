#include "FreeRTOS.h"
#include "task.h"

#include "h_soft_timer.h"
#include "taskmanager.h"

void task_cli_create(void);
void task_ir_create(void);
void task_log_create(void);
void task_ir_create(void);

void os_init(void)
{
  tm_system_init();
  h_soft_timer_init();
  task_log_create();
  task_cli_create();
  task_ir_create();
}

void os_start(void)
{
  vTaskStartScheduler();
}
