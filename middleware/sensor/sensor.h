#pragma once

#include <stdint.h>

#include "sys_msg.h"

#define SENSOR_OK           0
#define SENSOR_ERR_PARAM   -1
#define SENSOR_ERR_BUSY    -2
#define SENSOR_ERR_IO      -3

typedef struct
{
  float ax;
  float ay;
  float az;
  float temp_c;
  float gx;
  float gy;
  float gz;
} sensor_imu_sample_t;

typedef void (*sensor_imu_cb_t)(const sensor_imu_sample_t *sample, void *ctx);

int sensor_init(void);
void sensor_uninit(void);
void sensor_process(void);

int sensor_register_imu_cb(sensor_imu_cb_t cb, void *ctx);
int sensor_get_imu_sample(sensor_imu_sample_t *sample);
