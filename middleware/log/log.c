#include "log.h"

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_cfg.h"
#include "serial.h"
#include "sys_msg.h"
#include "taskmanager.h"

/* Max formatted log line length (middleware default; an app may override it
 * in app_cfg.h). */
#ifndef LOG_LINE_MAX
#define LOG_LINE_MAX 128U
#endif

static serial_t s_log_tx;
static uint8_t s_ready;

static uint8_t s_pending[LOG_LINE_MAX + 2U];
static size_t s_pending_len;

static void log_tx_isr(uart_id_t port, uart_event_t evt, void *ctx)
{
  BaseType_t hpw = pdFALSE;

  (void)port;
  (void)ctx;

  if (evt != UART_EVENT_TX_EMPTY) {
    return;
  }

  /* Ring drained: wake the task to push any remainder / next line. */
  tm_noti_from_isr(SYS_NODE_LOG, &hpw);
  portYIELD_FROM_ISR(hpw);
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

/* Send a full log line, then free its heap buffer. Whatever the TX ring cannot
 * accept right now is copied into the pending buffer for a later wake.
 * Returns the number of bytes left pending (0 = the line was fully sent). */
static size_t log_write(void *data, size_t len)
{
  size_t written = serial_write(&s_log_tx, (const uint8_t *)data, len);
  size_t remain = len - written;

  if (remain > sizeof(s_pending)) {
    remain = sizeof(s_pending);
  }

  if (remain > 0U) {
    memcpy(s_pending, (const uint8_t *)data + written, remain);
  }
  s_pending_len = remain;

  vPortFree(data);
  return remain;
}

/* Push the pending bytes into the TX ring; keep (compact to the front) whatever
 * the ring still cannot accept. Returns the number of bytes left pending. */
static size_t log_write_pending(void)
{
  size_t written = serial_write(&s_log_tx, s_pending, s_pending_len);
  size_t remain = s_pending_len - written;

  if (remain > 0U) {
    memmove(s_pending, s_pending + written, remain);
  }
  s_pending_len = remain;

  return remain;
}

int log_init(void)
{
  static const serial_cfg_t tx_cfg = {
      .isr_fn = log_tx_isr,
      .ctx = NULL,
  };

  if (s_ready != 0U) {
    return LOG_OK;
  }

  if (serial_register(LOG_SERIAL_TX, SERIAL_TYPE_TX, &tx_cfg, &s_log_tx) !=
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

  s_pending_len = 0U;
  s_ready = 0U;
  serial_unregister(&s_log_tx);
}

void log_process(void)
{
  sys_msg_t msg;

  for (;;) {
    tm_wait_notif();

    /* Finish a backpressured remainder before taking new lines. */
    if (s_pending_len != 0U && log_write_pending() > 0U) {
      continue;
    }

    /* Pending is empty: drain the queue until the ring backpressures. One wake
     * must service all queued lines because notifications coalesce. */
    while (tm_recv(&msg) == TM_OK) {
      if (msg.opcode != LOG_OPCODE_WRITE || msg.u.buf.data == NULL) {
        continue;
      }

      if (msg.u.buf.lenght == 0U) {
        vPortFree(msg.u.buf.data);
        continue;
      }

      if (log_write(msg.u.buf.data, (size_t)msg.u.buf.lenght) > 0U) {
        break; /* ring full: resume on next wake (TX_EMPTY / new message) */
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
