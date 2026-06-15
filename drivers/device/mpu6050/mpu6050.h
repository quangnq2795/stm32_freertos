#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  MPU6050_OK = 0,
  MPU6050_ERR_PARAM = -1,
  MPU6050_ERR_IO = -2,
  MPU6050_ERR_BUSY = -3,
  MPU6050_ERR_NOT_READY = -4,
} mpu6050_status_t;

typedef enum
{
  MPU6050_ACCEL_FS_2G = 0,
  MPU6050_ACCEL_FS_4G = 1,
  MPU6050_ACCEL_FS_8G = 2,
  MPU6050_ACCEL_FS_16G = 3,
} mpu6050_accel_fs_t;

typedef enum
{
  MPU6050_GYRO_FS_250DPS = 0,
  MPU6050_GYRO_FS_500DPS = 1,
  MPU6050_GYRO_FS_1000DPS = 2,
  MPU6050_GYRO_FS_2000DPS = 3,
} mpu6050_gyro_fs_t;

typedef struct
{
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t temp;
  int16_t gx;
  int16_t gy;
  int16_t gz;
} mpu6050_raw_t;

typedef struct
{
  float ax;
  float ay;
  float az;
  float temp_c;
  float gx;
  float gy;
  float gz;
} mpu6050_data_t;

typedef void (*mpu6050_evt_cb_t)(mpu6050_status_t status, void *ctx);

/*
 * Async MPU6050 driver on top of async I2C.
 * evt_cb is invoked from I2C ISR context when a transfer completes.
 */
void mpu6050_set_evt_cb(mpu6050_evt_cb_t cb, void *ctx);

mpu6050_status_t mpu6050_hw_init(void);
uint8_t mpu6050_hw_ready(void);

mpu6050_status_t mpu6050_write_reg_async(uint8_t reg, uint8_t value);
mpu6050_status_t mpu6050_read_reg_async(uint8_t reg, uint8_t *buf, size_t len);
mpu6050_status_t mpu6050_read_sample_async(uint8_t *buf);

void mpu6050_parse_burst(const uint8_t *buf, mpu6050_raw_t *raw);
void mpu6050_convert(const mpu6050_raw_t *raw,
                   mpu6050_accel_fs_t accel_fs,
                   mpu6050_gyro_fs_t gyro_fs,
                   mpu6050_data_t *out);
