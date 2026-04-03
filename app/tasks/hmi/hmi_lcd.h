#pragma once

#include "hmi_msg.h"

void hmi_lcd_init(void);
void hmi_lcd_on_msg(const hmi_msg_t *msg);
