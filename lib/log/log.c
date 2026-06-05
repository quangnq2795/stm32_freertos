#include "log.h"

#include <stdarg.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sys_msg.h"
#include "taskmanager.h"

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

static uint8_t s_ready;

void log_init(void)
{
  s_ready = 0U;
}

void log_set_ready(uint8_t ready)
{
  s_ready = ready;
}

static void log_post(char *data, size_t len)
{
  sys_msg_t msg = {0};

  msg.opcode = LOG_OPCODE_WRITE;
  msg.u.buf.lenght = (uint32_t)len;
  msg.u.buf.data = data;

  if (tm_send(SYS_NODE_LOG, &msg, pdMS_TO_TICKS(10)) != TM_OK) {
    vPortFree(data);
  }
}


void log_printf(const char *fmt, ...)
{
  char *buf;
  va_list ap;
  int n;
  size_t len;

  if (fmt == NULL || s_ready == 0U) {
    return;
  }

  buf = pvPortMalloc(LOG_LINE_MAX + 2U);
  if (buf == NULL) {
    return;
  }

  va_start(ap, fmt);
  n = vsnprintf(buf, LOG_LINE_MAX, fmt, ap);
  va_end(ap);

  if (n <= 0) {
    vPortFree(buf);
    return;
  }

  len = (size_t)n;
  if (len >= LOG_LINE_MAX) {
    len = LOG_LINE_MAX - 1U;
  }

  buf[len] = '\r';
  buf[len + 1U] = '\n';
  log_post(buf, len + 2U);
}
