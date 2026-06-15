#include "task_sensor.h"

#include "FreeRTOS.h"
#include "task.h"

#include "cli.h"
#include "log.h"
#include "sensor.h"
#include "taskmanager.h"

#define SENSOR_TASK_STACK_WORDS  512U
#define SENSOR_TASK_PRIO         (tskIDLE_PRIORITY + 2U)

static void cmd_imu(int argc, char **argv);

static void cmd_imu(int argc, char **argv)
{
  sensor_imu_sample_t sample;
  int ax_mg;
  int ay_mg;
  int az_mg;
  int gx_mdps;
  int gy_mdps;
  int gz_mdps;
  int temp_mc;

  (void)argc;
  (void)argv;

  if (sensor_get_imu_sample(&sample) != SENSOR_OK) {
    log_printf("IMU sample not ready");
    return;
  }

  ax_mg = (int)(sample.ax * 1000.0f);
  ay_mg = (int)(sample.ay * 1000.0f);
  az_mg = (int)(sample.az * 1000.0f);
  gx_mdps = (int)(sample.gx * 1000.0f);
  gy_mdps = (int)(sample.gy * 1000.0f);
  gz_mdps = (int)(sample.gz * 1000.0f);
  temp_mc = (int)(sample.temp_c * 1000.0f);

  log_printf("IMU ax=%d ay=%d az=%d mg gx=%d gy=%d gz=%d mdps T=%d mc",
             ax_mg, ay_mg, az_mg, gx_mdps, gy_mdps, gz_mdps, temp_mc);
}

static void sensor_task_init(void *ctx)
{
  (void)ctx;

  if (sensor_init() != SENSOR_OK) {
    return;
  }

  (void)cli_register_command("imu", cmd_imu);
}

static void sensor_task_uninit(void *ctx)
{
  (void)ctx;

  sensor_uninit();
}

static void sensor_task_handler(void *ctx)
{
  (void)ctx;

  sensor_process();
}

void task_sensor_create(void)
{
  static uint8_t s_started;

  const tm_task_cfg_t cfg = {
      .id = SYS_NODE_SENSOR,
      .name = "sensor",
      .ops = {
          .task_init = sensor_task_init,
          .task_uninit = sensor_task_uninit,
          .task_handler = sensor_task_handler,
      },
      .stack_words = SENSOR_TASK_STACK_WORDS,
      .priority = SENSOR_TASK_PRIO,
  };

  if (s_started != 0U) {
    return;
  }

  s_started = 1U;
  (void)tm_init(&cfg);
}
