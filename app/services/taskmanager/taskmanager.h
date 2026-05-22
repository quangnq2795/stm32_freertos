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
#define TM_MAX_LINKS             48U
#define TM_MAX_LISTEN_PER_TASK   8U
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
    TM_OK_FORWARDED = 1,
} tm_status_t;

typedef void (*tm_handler_fn)(const sys_msg_t *msg, void *ctx);

typedef struct
{
    sys_node_t from;
    uint16_t queue_len; /* 0 = TM_QUEUE_LENGTH */
} tm_listen_t;

typedef struct
{
    sys_node_t id;
    const char *name;

    /* Message-driven task: handler set, entry NULL. */
    tm_handler_fn handler;
    void *handler_ctx;

    /* Custom task body (e.g. CLI): entry set, handler NULL. */
    TaskFunction_t entry;
    void *entry_arg;

    uint16_t stack_words;
    UBaseType_t priority;

    const tm_listen_t *listen;
    uint8_t listen_count;
} tm_task_cfg_t;

/* Call once before any tm_init (e.g. in os_init). */
void tm_system_init(void);

/* Register id, queues/listeners, and create the FreeRTOS task. */
int tm_init(const tm_task_cfg_t *cfg);

int tm_listen(sys_node_t from_src, uint32_t queue_length);

int tm_send(sys_node_t to_id, sys_msg_t *msg, TickType_t timeout);

int tm_send_from_isr(sys_node_t to_id,
                     sys_msg_t *msg,
                     BaseType_t *hpw);

int tm_recv(sys_msg_t *msg, TickType_t timeout);

sys_node_t tm_self(void);

TaskHandle_t tm_handle(sys_node_t id);

#ifdef __cplusplus
}
#endif

#endif
