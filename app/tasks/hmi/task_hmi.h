#pragma once

#include "hmi_msg.h"

#include "FreeRTOS.h"

#include <stdbool.h>

bool hmi_cmd_send(const hmi_msg_t *msg, TickType_t ticks_to_wait);

void task_hmi_create(void);
