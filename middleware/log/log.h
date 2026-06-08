#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef LOG_LINE_MAX
#define LOG_LINE_MAX  128U
#endif

#define LOG_OK           0
#define LOG_ERR_BUSY    -2

int log_init(void);
void log_uninit(void);
void log_process(void);

/* Format log line (%s %d %u %x %X %02X %c %%), queue heap buffer, notify log task.
 * Appends "\r\n". */
void log_printf(const char *fmt, ...);
