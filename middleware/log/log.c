#include "log.h"

#include <stdarg.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "serial.h"
#include "sys_msg.h"
#include "taskmanager.h"

#ifndef LOG_SERIAL_TX
#define LOG_SERIAL_TX  SERIAL_PORT_2_TX
#endif

static serial_t s_log_tx;
static uint8_t s_ready;

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
                           uint32_t base, uint8_t uppercase, uint32_t width,
                           uint8_t zero_pad)
{
  char tmp[10];
  uint8_t n = 0U;
  uint32_t i;

  if (val == 0U) {
    tmp[n++] = '0';
  } else {
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
  }

  if (width > (uint32_t)n) {
    char pad = (zero_pad != 0U) ? '0' : ' ';

    for (i = 0U; i < (width - (uint32_t)n); i++) {
      pos = log_fmt_putc(out, cap, pos, pad);
    }
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

    uint8_t zero_pad = 0U;
    uint32_t width = 0U;

    if (*fmt == '0') {
      zero_pad = 1U;
      fmt++;
    }
    while ((*fmt >= '0') && (*fmt <= '9')) {
      width = (width * 10U) + (uint32_t)(*fmt - '0');
      fmt++;
    }

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
        pos = log_fmt_uint(out, cap, pos, (uint32_t)(-(v + 1)) + 1U, 10U, 0U,
                           width, zero_pad);
      } else {
        pos = log_fmt_uint(out, cap, pos, (uint32_t)v, 10U, 0U, width,
                           zero_pad);
      }
      break;
    }
    case 'u':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 10U, 0U,
                         width, zero_pad);
      break;
    case 'x':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 16U, 0U,
                         width, zero_pad);
      break;
    case 'X':
      pos = log_fmt_uint(out, cap, pos, va_arg(ap, unsigned int), 16U, 1U,
                         width, zero_pad);
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

static void log_handle_write(const sys_msg_t *msg)
{
  void *data;
  uint32_t len;

  if (msg == NULL) {
    return;
  }

  data = msg->u.buf.data;
  len = msg->u.buf.lenght;

  if (data != NULL && len > 0U) {
    const uint8_t *p = (const uint8_t *)data;
    size_t total = (size_t)len;
    size_t off = 0U;

    while (off < total) {
      size_t n = serial_write(&s_log_tx, p + off, total - off);

      if (n > 0U) {
        off += n;
      }

      if (off < total) {
        vTaskDelay(pdMS_TO_TICKS(50));
      }
    }
  }

  if (data != NULL) {
    vPortFree(data);
  }
}

int log_init(void)
{
  if (s_ready != 0U) {
    return LOG_OK;
  }

  if (serial_register(LOG_SERIAL_TX, SERIAL_TYPE_TX, NULL, &s_log_tx) !=
      SERIAL_OK) {
    return LOG_ERR_BUSY;
  }

  s_ready = 1U;
  return LOG_OK;
}

void log_uninit(void)
{
  if (s_ready == 0U) {
    return;
  }

  s_ready = 0U;
  serial_unregister(&s_log_tx);
}

void log_process(void)
{
  sys_msg_t msg;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      if (msg.opcode == LOG_OPCODE_WRITE) {
        log_handle_write(&msg);
      }
    }
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
