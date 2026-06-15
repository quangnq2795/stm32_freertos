#pragma once

#include "bsp_i2c_cfg.h"

/* MPU6050 on Arduino I2C header (I2C1 / PB8-PB9). AD0 = GND -> 0x68. */
#define BSP_MPU6050_I2C_ID          0U
#define BSP_MPU6050_I2C_ADDR        0x68U

#ifndef BSP_MPU6050_I2C_TRIALS
#define BSP_MPU6050_I2C_TRIALS      3U
#endif

#ifndef BSP_MPU6050_I2C_TIMEOUT_MS
#define BSP_MPU6050_I2C_TIMEOUT_MS  (BSP_I2C_TIMEOUT_MS * 2U)
#endif

#ifndef BSP_MPU6050_DEFAULT_ACCEL_FS
#define BSP_MPU6050_DEFAULT_ACCEL_FS  0U /* ±2 g */
#endif

#ifndef BSP_MPU6050_DEFAULT_GYRO_FS
#define BSP_MPU6050_DEFAULT_GYRO_FS   0U /* ±250 dps */
#endif

#ifndef BSP_MPU6050_DLPF_CFG
#define BSP_MPU6050_DLPF_CFG          0x03U /* ~44 Hz accel/gyro bandwidth. */
#endif

#ifndef BSP_MPU6050_SMPLRT_DIV
#define BSP_MPU6050_SMPLRT_DIV        0x07U /* 1 kHz / (1 + 7) = 125 Hz sample rate. */
#endif

#ifndef BSP_MPU6050_SAMPLE_MS
#define BSP_MPU6050_SAMPLE_MS         100U
#endif
