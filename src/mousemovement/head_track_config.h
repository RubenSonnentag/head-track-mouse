#pragma once

#include <Arduino.h>

namespace head_track_config {

constexpr uint8_t PIN_BUTTON_A2 = 1;
constexpr uint8_t PIN_TOUCH_SEND = 2;
constexpr uint8_t PIN_TOUCH_RECEIVE = 3;
constexpr uint8_t PIN_IMU_CS0 = 10;
constexpr uint8_t PIN_IMU_CS1 = 9;

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
constexpr uint32_t LOOP_INTERVAL_US = 4000;
constexpr uint16_t TOUCH_WARMUP_SAMPLES = 32;
constexpr uint16_t TOUCH_BASELINE_SAMPLES = 128;
constexpr uint16_t TOUCH_SAMPLE_TIMEOUT = 400;
constexpr uint16_t TOUCH_THRESHOLD_OFFSET = 80;
constexpr uint16_t TOUCH_RELEASE_OFFSET = 40;
constexpr uint8_t TOUCH_ACTIVATE_STABLE_SAMPLES = 3;
constexpr uint8_t TOUCH_RELEASE_STABLE_SAMPLES = 3;
constexpr float TOUCH_BASELINE_BLEND = 0.02f;

constexpr uint16_t IMU_TICK_SAMPLES = 128;
constexpr uint32_t CALIBRATION_SAMPLES_GYRO = 50000;
constexpr uint32_t CALIBRATION_SAMPLES_ACCEL = 20000;

constexpr double GYRO_SENSITIVITY = (1.0 / 512.0) * 1.45;
constexpr double GYRO_SENSITIVITY_X = GYRO_SENSITIVITY * 1.0;
constexpr double GYRO_SENSITIVITY_Y = GYRO_SENSITIVITY * 1.0;
constexpr double GYRO_SENSITIVITY_Z = GYRO_SENSITIVITY * 1.0;

constexpr float ACCEL_CORRECTION_SMOOTH = 50.0f;
constexpr float ACCEL_CORRECTION_RATE = 0.0007f;

constexpr uint32_t EEPROM_MAGIC = 0x48544D31UL;
constexpr uint16_t EEPROM_VERSION = 1;

}  // namespace head_track_config
