#include <string.h>

#include "taskmanager.h"

typedef struct
{
    sys_node_t id;
    TaskHandle_t task;
    QueueSetHandle_t set;
    uint8_t listen_count;
    tm_handler_fn handler;
    void *handler_ctx;
} tm_task_entry_t;

typedef struct
{
    uint8_t used;
    sys_node_t src;
    sys_node_t dst;
    QueueHandle_t queue;
} tm_link_t;

static tm_task_entry_t g_tasks[TM_MAX_TASK];
static tm_link_t g_links[TM_MAX_LINKS];

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

static tm_link_t *tm_find_link(sys_node_t src, sys_node_t dst)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_LINKS; i++) {
        if (g_links[i].used != 0U &&
            g_links[i].src == src &&
            g_links[i].dst == dst) {
            return &g_links[i];
        }
    }

    return NULL;
}

static tm_link_t *tm_alloc_link(void)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_LINKS; i++) {
        if (g_links[i].used == 0U) {
            return &g_links[i];
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

static int tm_listen_to(sys_node_t dst, sys_node_t src, uint32_t queue_length)
{
    tm_task_entry_t *task;
    tm_link_t *link;
    QueueHandle_t queue;

    if (dst == SYS_NODE_NONE || src == SYS_NODE_NONE || src == dst) {
        return TM_ERR_PARAM;
    }

    if (queue_length == 0U) {
        queue_length = TM_QUEUE_LENGTH;
    }

    task = tm_find_task(dst);
    if (task == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    if (task->listen_count >= TM_MAX_LISTEN_PER_TASK) {
        return TM_ERR_FULL;
    }

    if (tm_find_link(src, dst) != NULL) {
        return TM_ERR_PARAM;
    }

    link = tm_alloc_link();
    if (link == NULL) {
        return TM_ERR_FULL;
    }

    queue = xQueueCreate((UBaseType_t)queue_length, sizeof(sys_msg_t));
    if (queue == NULL) {
        return TM_ERR_FULL;
    }

    if (xQueueAddToSet(queue, task->set) != pdPASS) {
        vQueueDelete(queue);
        return TM_ERR_FULL;
    }

    link->used = 1U;
    link->src = src;
    link->dst = dst;
    link->queue = queue;
    task->listen_count++;

    return TM_OK;
}

static int tm_receive(sys_node_t dst, sys_msg_t *msg)
{
    tm_task_entry_t *task;
    QueueSetMemberHandle_t active;
    tm_link_t *link;
    uint32_t i;

    if (msg == NULL) {
        return TM_ERR_PARAM;
    }

    task = tm_find_task(dst);
    if (task == NULL || task->set == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    if (task->listen_count == 0U) {
        return TM_ERR_NOT_FOUND;
    }

    active = xQueueSelectFromSet(task->set, 0);
    if (active == NULL) {
        return TM_ERR_EMPTY;
    }

    for (i = 0U; i < TM_MAX_LINKS; i++) {
        if (g_links[i].used != 0U && g_links[i].queue == active) {
            link = &g_links[i];
            if (xQueueReceive(link->queue, msg, 0) != pdPASS) {
                return TM_ERR_EMPTY;
            }
            return TM_OK;
        }
    }

    return TM_ERR_NOT_FOUND;
}

static void tm_msg_loop(void *arg)
{
    tm_task_entry_t *self = (tm_task_entry_t *)arg;
    sys_msg_t msg;

    for (;;) {
        tm_wait_notif();
        while (tm_receive(self->id, &msg) == TM_OK) {
            if (self->handler != NULL) {
                self->handler(&msg, self->handler_ctx);
            }
        }
    }
}

void tm_system_init(void)
{
    uint32_t i;

    for (i = 0U; i < TM_MAX_TASK; i++) {
        g_tasks[i].id = SYS_NODE_NONE;
        g_tasks[i].task = NULL;
        g_tasks[i].set = NULL;
        g_tasks[i].listen_count = 0U;
        g_tasks[i].handler = NULL;
        g_tasks[i].handler_ctx = NULL;
    }

    for (i = 0U; i < TM_MAX_LINKS; i++) {
        g_links[i].used = 0U;
        g_links[i].src = SYS_NODE_NONE;
        g_links[i].dst = SYS_NODE_NONE;
        g_links[i].queue = NULL;
    }
}

int tm_init(const tm_task_cfg_t *cfg)
{
    tm_task_entry_t *entry;
    TaskFunction_t fn;
    void *arg;
    uint16_t stack;
    UBaseType_t prio;
    uint8_t i;
    BaseType_t created;

    if (cfg == NULL || cfg->id == SYS_NODE_NONE || cfg->id == SYS_NODE_ISR ||
        cfg->name == NULL) {
        return TM_ERR_PARAM;
    }

    if ((cfg->handler != NULL) && (cfg->entry != NULL)) {
        return TM_ERR_PARAM;
    }

    if ((cfg->handler == NULL) && (cfg->entry == NULL)) {
        return TM_ERR_PARAM;
    }

    if (tm_find_task(cfg->id) != NULL) {
        return TM_ERR_PARAM;
    }

    entry = tm_alloc_task();
    if (entry == NULL) {
        return TM_ERR_FULL;
    }

    entry->id = cfg->id;
    entry->handler = cfg->handler;
    entry->handler_ctx = cfg->handler_ctx;
    entry->listen_count = 0U;

    entry->set = xQueueCreateSet(
        (UBaseType_t)TM_MAX_LISTEN_PER_TASK * (UBaseType_t)TM_QUEUE_LENGTH);
    if (entry->set == NULL) {
        entry->id = SYS_NODE_NONE;
        return TM_ERR_FULL;
    }

    if (cfg->listen != NULL) {
        for (i = 0U; i < cfg->listen_count; i++) {
            uint32_t qlen = cfg->listen[i].queue_len;

            if (tm_listen_to(cfg->id,
                             cfg->listen[i].from,
                             qlen) != TM_OK) {
                entry->id = SYS_NODE_NONE;
                return TM_ERR_FULL;
            }
        }
    }

    if (cfg->id != SYS_NODE_ISR && tm_find_link(SYS_NODE_ISR, cfg->id) == NULL) {
        if (tm_listen_to(cfg->id, SYS_NODE_ISR, 0U) != TM_OK) {
            entry->id = SYS_NODE_NONE;
            return TM_ERR_FULL;
        }
    }

    stack = (cfg->stack_words != 0U) ? cfg->stack_words : TM_STACK_DEFAULT;
    prio = (cfg->priority != 0U) ? cfg->priority : TM_PRIO_DEFAULT;

    if (cfg->handler != NULL) {
        fn = tm_msg_loop;
        arg = entry;
    } else {
        fn = cfg->entry;
        arg = cfg->entry_arg;
    }

    created = xTaskCreate(fn,
                          cfg->name,
                          (configSTACK_DEPTH_TYPE)stack,
                          arg,
                          prio,
                          &entry->task);
    if (created != pdPASS) {
        entry->id = SYS_NODE_NONE;
        entry->task = NULL;
        return TM_ERR_FULL;
    }

    return TM_OK;
}

int tm_listen(sys_node_t from_src, uint32_t queue_length)
{
    sys_node_t self = tm_self();

    if (self == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    return tm_listen_to(self, from_src, queue_length);
}

int tm_send(sys_node_t to_id, sys_msg_t *msg, TickType_t timeout)
{
    tm_link_t *link;
    sys_node_t self;

    if (msg == NULL || to_id == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    self = tm_self();
    if (self == SYS_NODE_NONE) {
        return TM_ERR_PARAM;
    }

    msg->src = (uint32_t)self;

    link = tm_find_link(self, to_id);
    if (link == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    if (xQueueSend(link->queue, msg, timeout) != pdPASS) {
        return TM_ERR_FULL;
    }

    tm_noti(to_id);
    return TM_OK;
}

int tm_send_from_isr(sys_node_t to_id,
                     sys_msg_t *msg,
                     BaseType_t *hpw)
{
    tm_link_t *link;

    if (msg == NULL || to_id == SYS_NODE_NONE || to_id == SYS_NODE_ISR) {
        return TM_ERR_PARAM;
    }

    msg->src = (uint32_t)SYS_NODE_ISR;

    link = tm_find_link(SYS_NODE_ISR, to_id);
    if (link == NULL) {
        return TM_ERR_NOT_FOUND;
    }

    if (xQueueSendFromISR(link->queue, msg, hpw) != pdPASS) {
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
