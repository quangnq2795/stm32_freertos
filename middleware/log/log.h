#pragma once

#include <stddef.h>
#include <stdint.h>

#include "app_cfg.h"

#define LOG_OK           0
#define LOG_ERR_BUSY    -2

int log_init(void);
void log_uninit(void);
void log_process(void);

/* Format log line (%s %d %u %x %X %02X %c %%), queue heap buffer, notify log task.
 * Appends "\r\n". */
void log_printf(const char *fmt, ...);
