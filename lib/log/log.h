#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef LOG_LINE_MAX
#define LOG_LINE_MAX  128U
#endif

void log_init(void);
void log_set_ready(uint8_t ready);

/* Format log line (%s %d %u %x %X %c %%), queue heap buffer, notify log task.
 * Appends "\r\n". */
void log_printf(const char *fmt, ...);
