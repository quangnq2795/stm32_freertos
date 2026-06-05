#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "sys_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TM_MAX_TASK              16U
#define TM_QUEUE_LENGTH          8U

#define TM_STACK_DEFAULT         384U
#define TM_PRIO_DEFAULT          (tskIDLE_PRIORITY + 1U)

typedef enum
{
    TM_OK = 0,
    TM_ERR_PARAM = -1,
    TM_ERR_FULL = -2,
    TM_ERR_EMPTY = -3,
    TM_ERR_NOT_FOUND = -4,
} tm_status_t;

typedef void (*tm_task_init_fn)(void *ctx);
typedef void (*tm_task_uninit_fn)(void *ctx);
typedef void (*tm_task_handler_fn)(void *ctx);

typedef struct
{
    tm_task_init_fn task_init;
    tm_task_uninit_fn task_uninit;
    tm_task_handler_fn task_handler;
} tm_task_ops_t;

typedef struct
{
    sys_node_t id;
    const char *name;

    tm_task_ops_t ops;
    void *ctx;

    uint16_t stack_words;
    UBaseType_t priority;

    /* Per-task inbox depth; 0 = TM_QUEUE_LENGTH. */
    uint16_t queue_len;
} tm_task_cfg_t;

/* Call once before any tm_init (e.g. in os_init). */
void tm_system_init(void);

/* Register id, inbox queue, and create the FreeRTOS task. */
int tm_init(const tm_task_cfg_t *cfg);

int tm_send(sys_node_t to_id, sys_msg_t *msg, TickType_t timeout);

/* msg->src is always set to SYS_NODE_ISR. */
int tm_send_from_isr(sys_node_t to_id,
                     sys_msg_t *msg,
                     BaseType_t *hpw);

/* Non-blocking: returns TM_ERR_EMPTY immediately if no message. */
int tm_recv(sys_msg_t *msg);

/* Block until tm_send / tm_send_from_isr notifies this task. */
void tm_wait_notif(void);

void tm_noti(sys_node_t dst);

void tm_noti_from_isr(sys_node_t dst, BaseType_t *hpw);

sys_node_t tm_self(void);

TaskHandle_t tm_handle(sys_node_t id);

#ifdef __cplusplus
}
#endif

#endif
