#include "log.h"

#include <stdarg.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sys_msg.h"
#include "taskmanager.h"

static uint8_t s_ready;

void log_init(void)
{
  s_ready = 0U;
}

void log_set_ready(uint8_t ready)
{
  s_ready = ready;
}

static size_t log_fmt_putc(char *out, size_t cap, size_t pos, char c)
{
  if (pos < cap) {
    out[pos] = c;
    return pos + 1U;
  }

  return pos;
}

static size_t log_fmt_str(char *out, size_t cap, size_t pos, const char *s)
{
  if (s == NULL) {
    s = "(null)";
  }

  while (*s != '\0') {
    pos = log_fmt_putc(out, cap, pos, *s++);
  }

  return pos;
}

static size_t log_fmt_uint(char *out, size_t cap, size_t pos, uint32_t val,
                           uint32_t base, uint8_t uppercase)
{
  char tmp[10];
  uint8_t n = 0U;

  if (val == 0U) {
    return log_fmt_putc(out, cap, pos, '0');
  }

  while (val > 0U) {
    uint32_t digit = val % base;

    if (digit < 10U) {
      tmp[n++] = (char)('0' + digit);
    } else {
      tmp[n++] = (char)((uppercase != 0U) ? ('A' + digit - 10U)
                                          : ('a' + digit - 10U));
    }
    val /= base;
  }

  while (n > 0U) {
    pos = log_fmt_putc(out, cap, pos, tmp[--n]);
  }

  return pos;
}

static int log_vformat(char *out, size_t cap, const char *fmt, va_list ap)
{
  size_t pos = 0U;

  if (out == NULL || cap == 0U || fmt == NULL) {
    return -1;
  }

  while (*fmt != '\0') {
    if (*fmt != '%') {
      pos = log_fmt_putc(out, cap, pos, *fmt++);
      continue;
    }

    fmt++;
    if (*fmt == '\0') {
      break;
    }

    switch (*fmt) {
    case 's':
      pos = log_fmt_str(out, cap, pos, va_arg(ap, const char *));
      break;
    case 'd': {
      int v = va_arg(ap, int);

      if (v < 0) {
        pos = log_fmt_putc(out, cap, pos, '-');
        pos = log_fmt_uint(out, cap, pos, (uint32_t)(-(v + 1)) + 1U, 10U, 0U);
      } else {
        pos = log_fmt_uint(out, cap, pos, (uint32_t)v, 10U, 0U);
      }
      break;
    }
    case 'u':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 10U, 0U);
      break;
    case 'x':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 16U, 0U);
      break;
    case 'X':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 16U, 1U);
      break;
    case 'c':
      pos = log_fmt_putc(out, cap, pos, (char)va_arg(ap, int));
      break;
    case '%':
      pos = log_fmt_putc(out, cap, pos, '%');
      break;
    default:
      pos = log_fmt_putc(out, cap, pos, '%');
      pos = log_fmt_putc(out, cap, pos, *fmt);
      break;
    }

    fmt++;
  }

  if (pos < cap) {
    out[pos] = '\0';
  } else {
    out[cap - 1U] = '\0';
  }

  return (int)pos;
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
  n = log_vformat(buf, LOG_LINE_MAX, fmt, ap);
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
