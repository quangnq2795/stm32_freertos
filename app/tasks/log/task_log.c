#include "task_log.h"

#include "FreeRTOS.h"
#include "task.h"

#include "log.h"
#include "serial.h"
#include "sys_msg.h"
#include "taskmanager.h"

#ifndef LOG_SERIAL_TX
#define LOG_SERIAL_TX  SERIAL_PORT_2_TX
#endif

#define LOG_TASK_STACK_WORDS  384U
#define LOG_TASK_PRIO         (tskIDLE_PRIORITY + 1U)

static serial_t s_log_tx;

static int log_serial_setup(void)
{
  if (serial_register(LOG_SERIAL_TX, SERIAL_TYPE_TX, NULL, &s_log_tx) !=
      SERIAL_OK) {
    return SERIAL_ERR_BUSY;
  }

  return SERIAL_OK;
}

static void log_task_init(void *ctx)
{
  (void)ctx;

  log_init();

  if (log_serial_setup() != SERIAL_OK) {
    return;
  }

  log_set_ready(1U);
  log_printf("log task ready");
}

static void log_task_uninit(void *ctx)
{
  (void)ctx;

  log_set_ready(0U);
  serial_unregister(&s_log_tx);
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
    (void)serial_write(&s_log_tx, data, (size_t)len);
  }

  if (data != NULL) {
    vPortFree(data);
  }
}

static void log_task_handler(void *ctx)
{
  sys_msg_t msg;

  (void)ctx;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      switch (msg.opcode) {
      case LOG_OPCODE_WRITE:
        log_handle_write(&msg);
        break;
      default:
        break;
      }
    }
  }
}

void task_log_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_LOG,
      .name = "log",
      .ops = {
          .task_init = log_task_init,
          .task_uninit = log_task_uninit,
          .task_handler = log_task_handler,
      },
      .stack_words = LOG_TASK_STACK_WORDS,
      .priority = LOG_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }

  s_started = 1U;
  (void)tm_init(&cfg);
}
