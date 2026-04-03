#include "logging.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

void log_prefix(const char* level) {
  Serial.printf("[%10lu] [%s] ", millis(), level);
}

void log_vprintf(const char* level, const char* format, va_list args) {
  char buffer[192];
  vsnprintf(buffer, sizeof(buffer), format, args);
  log_prefix(level);
  Serial.print(buffer);
  Serial.print("\r\n");
}

}  // namespace

void log_begin() {
  log_info(F("Serial logging aktiv."));
}

void log_info(const __FlashStringHelper* message) {
  log_prefix("INFO");
  Serial.print(message);
  Serial.print("\r\n");
}

void log_infof(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_vprintf("INFO", format, args);
  va_end(args);
}

void log_error(const __FlashStringHelper* message) {
  log_prefix("ERROR");
  Serial.print(message);
  Serial.print("\r\n");
}

void log_errorf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_vprintf("ERROR", format, args);
  va_end(args);
}

void log_button_pressed(const char* context) {
  log_infof("Button A2 gedrueckt (%s)", context);
}

void log_state_change(const char* state) {
  log_infof("State -> %s", state);
}

void log_calibration_step(const char* step) {
  log_infof("Kalibrierung: %s", step);
}

void log_imu_status(uint8_t cs_pin, uint8_t who_am_i, uint8_t ctrl1_xl, uint8_t ctrl2_g, bool ok) {
  log_infof("IMU cs=%u whoami=0x%02X xl=0x%02X g=0x%02X status=%s", cs_pin, who_am_i, ctrl1_xl, ctrl2_g,
            ok ? "ok" : "fehler");
}

void log_runtime_telemetry(const Vector& gyro, const Vector& accel, double move_x, double move_y, bool active) {
  static elapsedMillis elapsed_since_log;
  if (elapsed_since_log < 250) {
    return;
  }
  elapsed_since_log = 0;

  log_infof(
      "telemetry mode=%s gyro[x=%.2f y=%.2f z=%.2f] accel[x=%.2f y=%.2f z=%.2f] mouse[x=%.2f y=%.2f]",
      active ? "active" : "paused", gyro.x, gyro.y, gyro.z, accel.x, accel.y, accel.z, move_x, move_y);
}
