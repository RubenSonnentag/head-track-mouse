#pragma once

#include <Arduino.h>

#include "alpakka_math.h"

void log_begin();
void log_info(const __FlashStringHelper* message);
void log_infof(const char* format, ...);
void log_error(const __FlashStringHelper* message);
void log_errorf(const char* format, ...);
void log_button_pressed(const char* context);
void log_state_change(const char* state);
void log_calibration_step(const char* step);
void log_imu_status(uint8_t cs_pin, uint8_t who_am_i, uint8_t ctrl1_xl, uint8_t ctrl2_g, bool ok);
void log_runtime_telemetry(const Vector& gyro, const Vector& accel, double move_x, double move_y, bool active);
