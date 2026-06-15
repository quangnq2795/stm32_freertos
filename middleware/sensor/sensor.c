#include "sensor.h"

#include "sensor_mpu6050.h"
#include "taskmanager.h"

int sensor_init(void)
{
  sensor_mpu6050_init();
  return SENSOR_OK;
}

void sensor_uninit(void)
{
}

void sensor_process(void)
{
  sys_msg_t msg;

  for (;;) {
    tm_wait_notif();
    while (tm_recv(&msg) == TM_OK) {
      sensor_mpu6050_on_msg(&msg);
    }
  }
}

int sensor_register_imu_cb(sensor_imu_cb_t cb, void *ctx)
{
  if (cb == NULL) {
    return SENSOR_ERR_PARAM;
  }

  sensor_mpu6050_register_imu_cb(cb, ctx);
  return SENSOR_OK;
}

int sensor_get_imu_sample(sensor_imu_sample_t *sample)
{
  return sensor_mpu6050_get_sample(sample);
}
