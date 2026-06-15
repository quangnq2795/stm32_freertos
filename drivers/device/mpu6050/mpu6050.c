#include "mpu6050.h"

#include "bsp_mpu6050_cfg.h"
#include "i2c.h"
#include "mpu6050_regs.h"

static mpu6050_evt_cb_t s_evt_cb;
static void *s_evt_ctx;
static uint8_t s_hw_ready;
static uint8_t s_busy;
static uint8_t s_tx_byte;

static int16_t mpu6050_be16(const uint8_t *p)
{
  return (int16_t)((uint16_t)p[0] << 8 | (uint16_t)p[1]);
}

static mpu6050_status_t mpu6050_map_i2c_status(i2c_drv_status_t st)
{
  switch (st) {
  case I2C_DRV_OK:
    return MPU6050_OK;
  case I2C_DRV_ERR_BUSY:
    return MPU6050_ERR_BUSY;
  case I2C_DRV_ERR_PARAM:
    return MPU6050_ERR_PARAM;
  default:
    return MPU6050_ERR_IO;
  }
}

static void mpu6050_i2c_done(i2c_id_t id, i2c_drv_status_t status, void *ctx)
{
  mpu6050_evt_cb_t cb;
  void *cb_ctx;
  mpu6050_status_t st;

  (void)id;
  (void)ctx;

  cb = s_evt_cb;
  cb_ctx = s_evt_ctx;
  s_busy = 0U;
  st = mpu6050_map_i2c_status(status);

  if (cb != NULL) {
    cb(st, cb_ctx);
  }
}

static mpu6050_status_t mpu6050_start_mem_write(uint8_t reg, const uint8_t *buf,
                                                size_t len)
{
  i2c_drv_status_t st;

  if (s_hw_ready == 0U) {
    return MPU6050_ERR_NOT_READY;
  }
  if (s_busy != 0U) {
    return MPU6050_ERR_BUSY;
  }
  if (buf == NULL || len == 0U) {
    return MPU6050_ERR_PARAM;
  }

  s_busy = 1U;
  st = i2c_mem_write_async(BSP_MPU6050_I2C_ID, BSP_MPU6050_I2C_ADDR, reg,
                           I2C_MEMADD_SIZE_8BIT, buf, len, mpu6050_i2c_done,
                           NULL);
  if (st != I2C_DRV_OK) {
    s_busy = 0U;
    return mpu6050_map_i2c_status(st);
  }

  return MPU6050_OK;
}

static mpu6050_status_t mpu6050_start_mem_read(uint8_t reg, uint8_t *buf,
                                               size_t len)
{
  i2c_drv_status_t st;

  if (s_hw_ready == 0U) {
    return MPU6050_ERR_NOT_READY;
  }
  if (s_busy != 0U) {
    return MPU6050_ERR_BUSY;
  }
  if (buf == NULL || len == 0U) {
    return MPU6050_ERR_PARAM;
  }

  s_busy = 1U;
  st = i2c_mem_read_async(BSP_MPU6050_I2C_ID, BSP_MPU6050_I2C_ADDR, reg,
                          I2C_MEMADD_SIZE_8BIT, buf, len, mpu6050_i2c_done,
                          NULL);
  if (st != I2C_DRV_OK) {
    s_busy = 0U;
    return mpu6050_map_i2c_status(st);
  }

  return MPU6050_OK;
}

void mpu6050_set_evt_cb(mpu6050_evt_cb_t cb, void *ctx)
{
  s_evt_cb = cb;
  s_evt_ctx = ctx;
}

mpu6050_status_t mpu6050_hw_init(void)
{
  if (s_hw_ready != 0U) {
    return MPU6050_OK;
  }

  i2c_init(BSP_MPU6050_I2C_ID);

  if (i2c_is_device_ready(BSP_MPU6050_I2C_ID, BSP_MPU6050_I2C_ADDR,
                          BSP_MPU6050_I2C_TRIALS) != I2C_DRV_OK) {
    return MPU6050_ERR_IO;
  }

  s_hw_ready = 1U;
  return MPU6050_OK;
}

uint8_t mpu6050_hw_ready(void)
{
  return s_hw_ready;
}

mpu6050_status_t mpu6050_write_reg_async(uint8_t reg, uint8_t value)
{
  s_tx_byte = value;
  return mpu6050_start_mem_write(reg, &s_tx_byte, 1U);
}

mpu6050_status_t mpu6050_read_reg_async(uint8_t reg, uint8_t *buf, size_t len)
{
  return mpu6050_start_mem_read(reg, buf, len);
}

mpu6050_status_t mpu6050_read_sample_async(uint8_t *buf)
{
  return mpu6050_start_mem_read(MPU6050_REG_ACCEL_XOUT_H, buf,
                                MPU6050_RAW_BURST_LEN);
}

void mpu6050_parse_burst(const uint8_t *buf, mpu6050_raw_t *raw)
{
  if (buf == NULL || raw == NULL) {
    return;
  }

  raw->ax = mpu6050_be16(&buf[0]);
  raw->ay = mpu6050_be16(&buf[2]);
  raw->az = mpu6050_be16(&buf[4]);
  raw->temp = mpu6050_be16(&buf[6]);
  raw->gx = mpu6050_be16(&buf[8]);
  raw->gy = mpu6050_be16(&buf[10]);
  raw->gz = mpu6050_be16(&buf[12]);
}

static float mpu6050_accel_lsb_per_g(mpu6050_accel_fs_t fs)
{
  switch (fs) {
  case MPU6050_ACCEL_FS_2G:
    return 16384.0f;
  case MPU6050_ACCEL_FS_4G:
    return 8192.0f;
  case MPU6050_ACCEL_FS_8G:
    return 4096.0f;
  case MPU6050_ACCEL_FS_16G:
    return 2048.0f;
  default:
    return 16384.0f;
  }
}

static float mpu6050_gyro_lsb_per_dps(mpu6050_gyro_fs_t fs)
{
  switch (fs) {
  case MPU6050_GYRO_FS_250DPS:
    return 131.0f;
  case MPU6050_GYRO_FS_500DPS:
    return 65.5f;
  case MPU6050_GYRO_FS_1000DPS:
    return 32.8f;
  case MPU6050_GYRO_FS_2000DPS:
    return 16.4f;
  default:
    return 131.0f;
  }
}

void mpu6050_convert(const mpu6050_raw_t *raw,
                     mpu6050_accel_fs_t accel_fs,
                     mpu6050_gyro_fs_t gyro_fs,
                     mpu6050_data_t *out)
{
  float accel_lsb;
  float gyro_lsb;

  if (raw == NULL || out == NULL) {
    return;
  }

  accel_lsb = mpu6050_accel_lsb_per_g(accel_fs);
  gyro_lsb = mpu6050_gyro_lsb_per_dps(gyro_fs);

  out->ax = (float)raw->ax / accel_lsb;
  out->ay = (float)raw->ay / accel_lsb;
  out->az = (float)raw->az / accel_lsb;
  out->temp_c = ((float)raw->temp / 340.0f) + 36.53f;
  out->gx = (float)raw->gx / gyro_lsb;
  out->gy = (float)raw->gy / gyro_lsb;
  out->gz = (float)raw->gz / gyro_lsb;
}
