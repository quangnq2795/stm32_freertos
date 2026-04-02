#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#define configUSE_PREEMPTION                    1
/* Must match SYSCLK after SystemClock_Config in bsp.c (120 MHz for NUCLEO-F207ZG). */
#define configCPU_CLOCK_HZ                      (120000000UL)
#define configTICK_RATE_HZ                      (1000U)
#define configMAX_PRIORITIES                    8
#define configMINIMAL_STACK_SIZE                128
#define configTOTAL_HEAP_SIZE                   (20 * 1024)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configUSE_MUTEXES                       1
#define configUSE_TIMERS                        0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configQUEUE_REGISTRY_SIZE               0

#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetSchedulerState          1

#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler
#define configASSERT(x)                         do { if ((x) == 0) { portDISABLE_INTERRUPTS(); for (;;) { } } } while (0)

#endif
