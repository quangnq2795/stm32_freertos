#ifndef SYS_MSG_H
#define SYS_MSG_H

#include <stdint.h>

typedef enum
{
    SYS_MSG_TYPE_READ = 0,
    SYS_MSG_TYPE_WRITE,
    SYS_MSG_TYPE_READ_RESPONSE,
} sys_msg_type_t;

typedef enum
{
    SYS_NODE_NONE = 0,
    SYS_NODE_ISR, /* Virtual node: ISR -> task via tm_send_from_isr (not a task). */
    SYS_NODE_CLI,
    SYS_NODE_BLE,
    SYS_NODE_SENSOR,
    SYS_NODE_AUDIO,
    SYS_NODE_MOTOR,
    SYS_NODE_CONTROL,
    SYS_NODE_IR,
    SYS_NODE_LOG,
} sys_node_t;

/* SYS_NODE_CLI opcodes (task_cli). */
#define CLI_OPCODE_RX  0U

/* SYS_NODE_IR opcodes (task_ir). */
#define IR_OPCODE_RX  0U
#define IR_OPCODE_TX  1U

/* SYS_NODE_LOG opcodes (task_log). */
#define LOG_OPCODE_WRITE  0U /* u.buf: heap log line; task writes then vPortFree(data). */

/* IR_OPCODE_RX: arg.param1 = ir_rx_channel_id_t */
/* IR_OPCODE_TX: arg.param1 = ir_tx channel, param2 = NEC address, param3 = NEC command,
 *               param4 = repeat (0 = full frame, non-zero = repeat code) */

typedef struct sys_msg_args_t
{
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
} sys_msg_args_t;

typedef struct sys_msg_buf_t
{
    uint32_t lenght;
    void *data;
} sys_msg_buf_t;

typedef struct sys_msg_t
{
    uint32_t id;
    uint32_t type;
    uint32_t src;
    uint32_t opcode;
    union
    {
        sys_msg_args_t arg;
        sys_msg_buf_t buf;
    } u;
} sys_msg_t;

#endif
