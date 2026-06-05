#include <string.h>

#include "taskmanager.h"

typedef struct
{
    sys_node_t id;
    TaskHandle_t task;
    QueueHandle_t queue;
    tm_task_ops_t ops;
    void *ctx;
} tm_task_entry_t;

static tm_task_entry_t g_tasks[TM_MAX_TASK];

static tm_task_entry_t *tm_find_task(sys_node_t id)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_TASK; i++) {
        if (g_tasks[i].id == id) {
            return &g_tasks[i];
        }
    }

    return NULL;
}

static tm_task_entry_t *tm_alloc_task(void)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_TASK; i++) {
        if (g_tasks[i].id == SYS_NODE_NONE) {
            return &g_tasks[i];
        }
    }

    return NULL;
}

void tm_noti(sys_node_t dst)
{
    tm_task_entry_t *task = tm_find_task(dst);

    if (task != NULL && task->task != NULL) {
        (void)xTaskNotify(task->task, 0, eIncrement);
    }
}

void tm_noti_from_isr(sys_node_t dst, BaseType_t *hpw)
{
    tm_task_entry_t *task = tm_find_task(dst);

    if (task != NULL && task->task != NULL) {
        (void)xTaskNotifyFromISR(task->task, 0, eIncrement, hpw);
    }
}

static int tm_receive(sys_node_t dst, sys_msg_t *msg)
{
    tm_task_entry_t *task;

    if (msg == NULL) {
        return TM_ERR_PARAM;
    }

    task = tm_find_task(dst);
    if (task == NULL || task->queue == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    if (xQueueReceive(task->queue, msg, 0) != pdPASS) {
        return TM_ERR_EMPTY;
    }

    return TM_OK;
}

static void tm_default_loop(void *arg)
{
    tm_task_entry_t *self = (tm_task_entry_t *)arg;

    if (self->ops.task_init != NULL) {
        self->ops.task_init(self->ctx);
    }

    if (self->ops.task_handler != NULL) {
        self->ops.task_handler(self->ctx);
    }

    if (self->ops.task_uninit != NULL) {
        self->ops.task_uninit(self->ctx);
    }
}

void tm_system_init(void)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_TASK; i++) {
        g_tasks[i].id = SYS_NODE_NONE;
        g_tasks[i].task = NULL;
        g_tasks[i].queue = NULL;
        g_tasks[i].ops.task_init = NULL;
        g_tasks[i].ops.task_uninit = NULL;
        g_tasks[i].ops.task_handler = NULL;
        g_tasks[i].ctx = NULL;
    }
}

int tm_init(const tm_task_cfg_t *cfg)
{
    tm_task_entry_t *entry;
    uint16_t stack;
    UBaseType_t prio;
    uint32_t queue_len;
    BaseType_t created;

    if (cfg == NULL || cfg->id == SYS_NODE_NONE || cfg->id == SYS_NODE_ISR ||
        cfg->name == NULL) {
        return TM_ERR_PARAM;
    }

    if (cfg->ops.task_handler == NULL) {
        return TM_ERR_PARAM;
    }

    if (tm_find_task(cfg->id) != NULL) {
        return TM_ERR_PARAM;
    }

    entry = tm_alloc_task();
    if (entry == NULL) {
        return TM_ERR_FULL;
    }

    queue_len = (cfg->queue_len != 0U) ? cfg->queue_len : TM_QUEUE_LENGTH;

    entry->id = cfg->id;
    entry->ops = cfg->ops;
    entry->ctx = cfg->ctx;
    entry->queue = xQueueCreate((UBaseType_t)queue_len, sizeof(sys_msg_t));
    if (entry->queue == NULL) {
        entry->id = SYS_NODE_NONE;
        return TM_ERR_FULL;
    }

    stack = (cfg->stack_words != 0U) ? cfg->stack_words : TM_STACK_DEFAULT;
    prio = (cfg->priority != 0U) ? cfg->priority : TM_PRIO_DEFAULT;

    created = xTaskCreate(tm_default_loop,
                          cfg->name,
                          (configSTACK_DEPTH_TYPE)stack,
                          entry,
                          prio,
                          &entry->task);
    if (created != pdPASS) {
        vQueueDelete(entry->queue);
        entry->id = SYS_NODE_NONE;
        entry->task = NULL;
        entry->queue = NULL;
        entry->ops.task_init = NULL;
        entry->ops.task_uninit = NULL;
        entry->ops.task_handler = NULL;
        entry->ctx = NULL;
        return TM_ERR_FULL;
    }

    return TM_OK;
}

int tm_send(sys_node_t to_id, sys_msg_t *msg, TickType_t timeout)
{
    tm_task_entry_t *dst;
    sys_node_t self;

    if (msg == NULL || to_id == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    self = tm_self();
    if (self == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    dst = tm_find_task(to_id);
    if (dst == NULL || dst->queue == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    msg->src = (uint32_t)self;

    if (xQueueSend(dst->queue, msg, timeout) != pdPASS) {
        return TM_ERR_FULL;
    }

    tm_noti(to_id);
    return TM_OK;
}

int tm_send_from_isr(sys_node_t to_id,
                     sys_msg_t *msg,
                     BaseType_t *hpw)
{
    tm_task_entry_t *dst;

    if (msg == NULL || to_id == SYS_NODE_NONE || to_id == SYS_NODE_ISR) {
        return TM_ERR_PARAM;
    }

    dst = tm_find_task(to_id);
    if (dst == NULL || dst->queue == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    msg->src = (uint32_t)SYS_NODE_ISR;

    if (xQueueSendFromISR(dst->queue, msg, hpw) != pdPASS) {
        return TM_ERR_FULL;
    }

    tm_noti_from_isr(to_id, hpw);
    return TM_OK;
}

int tm_recv(sys_msg_t *msg)
{
    sys_node_t self = tm_self();

    if (self == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    return tm_receive(self, msg);
}

void tm_wait_notif(void)
{
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

sys_node_t tm_self(void)
{
    TaskHandle_t me = xTaskGetCurrentTaskHandle();
    uint32_t i;

    for (i = 0U; i < TM_MAX_TASK; i++) {
        if (g_tasks[i].id != SYS_NODE_NONE && g_tasks[i].task == me) {
            return g_tasks[i].id;
        }
    }

    return SYS_NODE_NONE;
}

TaskHandle_t tm_handle(sys_node_t id)
{
    tm_task_entry_t *task = tm_find_task(id);

    if (task == NULL) {
        return NULL;
    }

    return task->task;
}
