#pragma once

#include <Arduino.h>
#include "alpakka_math.h"

struct ImuCalibrationData {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  double offset_gyro_0_x;
  double offset_gyro_0_y;
  double offset_gyro_0_z;
  double offset_gyro_1_x;
  double offset_gyro_1_y;
  double offset_gyro_1_z;
  double offset_accel_0_x;
  double offset_accel_0_y;
  double offset_accel_0_z;
  double offset_accel_1_x;
  double offset_accel_1_y;
  double offset_accel_1_z;
};

bool imu_init();
void imu_load_calibration();
bool imu_is_ready();
bool imu_has_calibration();
bool imu_calibrate();
Vector imu_read_gyro();
Vector imu_read_accel();
void imu_print_status();
