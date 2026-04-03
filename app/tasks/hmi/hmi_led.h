#pragma once

#include "hmi_msg.h"

void hmi_led_init(void);
void hmi_led_on_msg(const hmi_msg_t *msg);
