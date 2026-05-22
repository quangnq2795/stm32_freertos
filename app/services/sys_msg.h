#ifndef SYS_MSG_H
#define SYS_MSG_H

#include <stdint.h>

#ifndef SYS_MSG_LCD_TEXT_MAX
#define SYS_MSG_LCD_TEXT_MAX 48U
#endif

typedef enum
{
    SYS_MSG_TYPE_READ = 0,
    SYS_MSG_TYPE_WRITE,
    SYS_MSG_TYPE_READ_RESPONSE,
} sys_msg_type_t;

typedef enum
{
    SYS_NODE_NONE = 0,
    SYS_NODE_CLI,
    SYS_NODE_HMI,
    SYS_NODE_BLE,
    SYS_NODE_SENSOR,
    SYS_NODE_AUDIO,
    SYS_NODE_MOTOR,
    SYS_NODE_CONTROL,
} sys_node_t;

typedef enum
{
    SYS_NODE_HMI_OPCODE_LED_SET = 0,
    SYS_NODE_HMI_OPCODE_LED_BLINK,
    SYS_NODE_HMI_OPCODE_LCD_TEXT,
} sys_node_hmi_opcode_t;

typedef struct sys_msg_args_t
{
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
    uint32_t param4;
} sys_msg_args_t;

typedef struct sys_msg_lcd_t
{
    uint8_t row;
    uint8_t col;
    char text[SYS_MSG_LCD_TEXT_MAX];
} sys_msg_lcd_t;

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
    uint32_t dst;
    uint32_t opcode;
    union
    {
        sys_msg_args_t arg;
        sys_msg_lcd_t lcd;
        sys_msg_buf_t buf;
    } u;
} sys_msg_t;

#endif
