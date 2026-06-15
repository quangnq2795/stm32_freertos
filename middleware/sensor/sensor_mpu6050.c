#include "sensor_mpu6050.h"

#include "FreeRTOS.h"
#include "task.h"

#include "bsp_mpu6050_cfg.h"
#include "h_soft_timer.h"
#include "log.h"
#include "mpu6050.h"
#include "mpu6050_regs.h"
#include "sensor.h"
#include "sys_msg.h"
#include "taskmanager.h"

typedef enum
{
  SENSOR_MPU6050_OFF = 0,
  SENSOR_MPU6050_INIT_WAKE,
  SENSOR_MPU6050_INIT_WHOAMI,
  SENSOR_MPU6050_INIT_SMPLRT,
  SENSOR_MPU6050_INIT_CONFIG,
  SENSOR_MPU6050_INIT_GYRO,
  SENSOR_MPU6050_INIT_ACCEL,
  SENSOR_MPU6050_RUNNING,
  SENSOR_MPU6050_ERROR,
} sensor_mpu6050_state_t;

static sensor_mpu6050_state_t s_state = SENSOR_MPU6050_OFF;
static uint8_t s_reg_buf[MPU6050_RAW_BURST_LEN];
static int s_sample_timer_id = H_SOFT_TIMER_INVALID_ID;
static mpu6050_accel_fs_t s_accel_fs =
    (mpu6050_accel_fs_t)BSP_MPU6050_DEFAULT_ACCEL_FS;
static mpu6050_gyro_fs_t s_gyro_fs =
    (mpu6050_gyro_fs_t)BSP_MPU6050_DEFAULT_GYRO_FS;

static sensor_imu_sample_t s_latest;
static uint8_t s_has_sample;
static sensor_imu_cb_t s_imu_cb;
static void *s_imu_cb_ctx;

static void sensor_mpu6050_fail(const char *reason);
static void sensor_mpu6050_start_init(void);
static void sensor_mpu6050_on_i2c_done(mpu6050_status_t status);
static void sensor_mpu6050_start_sample(void);
static void sensor_mpu6050_publish_sample(const mpu6050_data_t *data);
static void sensor_mpu6050_sample_timer_cb(void *ctx);
static void sensor_mpu6050_arm_sample_timer(void);

static void sensor_mpu6050_on_evt(mpu6050_status_t status, void *ctx)
{
  sys_msg_t msg = {0};
  BaseType_t hpw = pdFALSE;

  (void)ctx;

  msg.opcode = SENSOR_OPCODE_I2C_DONE;
  msg.u.arg.param1 = (uint32_t)(int32_t)status;
  if (tm_send_from_isr(SYS_NODE_SENSOR, &msg, &hpw) == TM_OK) {
    portYIELD_FROM_ISR(hpw);
  }
}

static void sensor_mpu6050_sample_timer_cb(void *ctx)
{
  sys_msg_t msg = {0};
  BaseType_t hpw = pdFALSE;

  (void)ctx;

  s_sample_timer_id = H_SOFT_TIMER_INVALID_ID;

  msg.opcode = SENSOR_OPCODE_SAMPLE;
  if (tm_send_from_isr(SYS_NODE_SENSOR, &msg, &hpw) == TM_OK) {
    portYIELD_FROM_ISR(hpw);
  }

  sensor_mpu6050_arm_sample_timer();
}

static void sensor_mpu6050_fail(const char *reason)
{
  s_state = SENSOR_MPU6050_ERROR;
  if (s_sample_timer_id >= 0) {
    h_soft_timer_unregister(s_sample_timer_id);
    s_sample_timer_id = H_SOFT_TIMER_INVALID_ID;
  }
  log_printf("MPU6050 err: %s", reason);
}

static void sensor_mpu6050_start_init(void)
{
  s_state = SENSOR_MPU6050_INIT_WAKE;
  if (mpu6050_write_reg_async(MPU6050_REG_PWR_MGMT_1,
                              MPU6050_PWR_MGMT_1_CLKSEL_INTERNAL) !=
      MPU6050_OK) {
    sensor_mpu6050_fail("wake");
  }
}

static void sensor_mpu6050_arm_sample_timer(void)
{
  uint32_t period_us = (uint32_t)BSP_MPU6050_SAMPLE_MS * 1000U;

  if (s_sample_timer_id >= 0) {
    h_soft_timer_unregister(s_sample_timer_id);
    s_sample_timer_id = H_SOFT_TIMER_INVALID_ID;
  }

  s_sample_timer_id =
      h_soft_timer_register(period_us, sensor_mpu6050_sample_timer_cb, NULL);
  if (s_sample_timer_id < 0) {
    sensor_mpu6050_fail("sample timer");
  }
}

static void sensor_mpu6050_publish_sample(const mpu6050_data_t *data)
{
  if (data == NULL) {
    return;
  }

  s_latest.ax = data->ax;
  s_latest.ay = data->ay;
  s_latest.az = data->az;
  s_latest.temp_c = data->temp_c;
  s_latest.gx = data->gx;
  s_latest.gy = data->gy;
  s_latest.gz = data->gz;
  s_has_sample = 1U;

  if (s_imu_cb != NULL) {
    s_imu_cb(&s_latest, s_imu_cb_ctx);
  }
}

static void sensor_mpu6050_start_sample(void)
{
  if (s_state != SENSOR_MPU6050_RUNNING) {
    return;
  }

  if (mpu6050_read_sample_async(s_reg_buf) != MPU6050_OK) {
    return;
  }
}

static void sensor_mpu6050_on_i2c_done(mpu6050_status_t status)
{
  mpu6050_raw_t raw;
  mpu6050_data_t data;

  if (status != MPU6050_OK) {
    sensor_mpu6050_fail("i2c");
    return;
  }

  switch (s_state) {
  case SENSOR_MPU6050_INIT_WAKE:
    s_state = SENSOR_MPU6050_INIT_WHOAMI;
    if (mpu6050_read_reg_async(MPU6050_REG_WHO_AM_I, s_reg_buf, 1U) !=
        MPU6050_OK) {
      sensor_mpu6050_fail("whoami read");
    }
    break;

  case SENSOR_MPU6050_INIT_WHOAMI:
    if (s_reg_buf[0] != MPU6050_WHO_AM_I_VALUE) {
      sensor_mpu6050_fail("whoami value");
      break;
    }
    s_state = SENSOR_MPU6050_INIT_SMPLRT;
    if (mpu6050_write_reg_async(MPU6050_REG_SMPLRT_DIV,
                                BSP_MPU6050_SMPLRT_DIV) != MPU6050_OK) {
      sensor_mpu6050_fail("smplrt");
    }
    break;

  case SENSOR_MPU6050_INIT_SMPLRT:
    s_state = SENSOR_MPU6050_INIT_CONFIG;
    if (mpu6050_write_reg_async(MPU6050_REG_CONFIG, BSP_MPU6050_DLPF_CFG) !=
        MPU6050_OK) {
      sensor_mpu6050_fail("config");
    }
    break;

  case SENSOR_MPU6050_INIT_CONFIG:
    s_state = SENSOR_MPU6050_INIT_GYRO;
    if (mpu6050_write_reg_async(MPU6050_REG_GYRO_CONFIG,
                                (uint8_t)(s_gyro_fs << 3)) != MPU6050_OK) {
      sensor_mpu6050_fail("gyro fs");
    }
    break;

  case SENSOR_MPU6050_INIT_GYRO:
    s_state = SENSOR_MPU6050_INIT_ACCEL;
    if (mpu6050_write_reg_async(MPU6050_REG_ACCEL_CONFIG,
                                (uint8_t)(s_accel_fs << 3)) != MPU6050_OK) {
      sensor_mpu6050_fail("accel fs");
    }
    break;

  case SENSOR_MPU6050_INIT_ACCEL:
    s_state = SENSOR_MPU6050_RUNNING;
    sensor_mpu6050_arm_sample_timer();
    log_printf("MPU6050 ready");
    sensor_mpu6050_start_sample();
    break;

  case SENSOR_MPU6050_RUNNING:
    mpu6050_parse_burst(s_reg_buf, &raw);
    mpu6050_convert(&raw, s_accel_fs, s_gyro_fs, &data);
    sensor_mpu6050_publish_sample(&data);
    break;

  default:
    break;
  }
}

void sensor_mpu6050_register_imu_cb(sensor_imu_cb_t cb, void *ctx)
{
  s_imu_cb = cb;
  s_imu_cb_ctx = ctx;
}

int sensor_mpu6050_get_sample(sensor_imu_sample_t *sample)
{
  if (sample == NULL) {
    return SENSOR_ERR_PARAM;
  }
  if (s_has_sample == 0U) {
    return SENSOR_ERR_IO;
  }

  *sample = s_latest;
  return SENSOR_OK;
}

void sensor_mpu6050_init(void)
{
  mpu6050_status_t st;

  s_state = SENSOR_MPU6050_OFF;
  s_has_sample = 0U;
  s_sample_timer_id = H_SOFT_TIMER_INVALID_ID;

  mpu6050_set_evt_cb(sensor_mpu6050_on_evt, NULL);

  st = mpu6050_hw_init();
  if (st != MPU6050_OK) {
    sensor_mpu6050_fail("hw init");
    return;
  }

  sensor_mpu6050_start_init();
}

void sensor_mpu6050_on_msg(const sys_msg_t *msg)
{
  if (msg == NULL) {
    return;
  }

  switch (msg->opcode) {
  case SENSOR_OPCODE_I2C_DONE:
    sensor_mpu6050_on_i2c_done((mpu6050_status_t)(int32_t)msg->u.arg.param1);
    break;

  case SENSOR_OPCODE_SAMPLE:
    sensor_mpu6050_start_sample();
    break;

  default:
    break;
  }
}
