#pragma once

#include <Arduino.h>

struct MouseMovementPins {
  uint8_t calibrate_button;
  uint8_t touch_send;
  uint8_t touch_receive;
  uint8_t imu_cs0;
  uint8_t imu_cs1;
};

struct MouseMovementSettings {
  uint32_t button_debounce_ms;
  uint32_t loop_interval_us;
  double sensitivity_multiplier;
  bool logging_enabled;
};

struct MouseMovementConfig {
  MouseMovementPins pins;
  MouseMovementSettings settings;
};

class MouseMovement {
 public:
  void begin(const MouseMovementConfig& config);
  void process();
};
