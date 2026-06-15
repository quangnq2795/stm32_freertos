#pragma once

#include "sensor.h"
#include "sys_msg.h"

void sensor_mpu6050_init(void);
void sensor_mpu6050_on_msg(const sys_msg_t *msg);
void sensor_mpu6050_register_imu_cb(sensor_imu_cb_t cb, void *ctx);
int sensor_mpu6050_get_sample(sensor_imu_sample_t *sample);
