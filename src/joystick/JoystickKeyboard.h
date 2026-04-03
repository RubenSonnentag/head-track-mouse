#pragma once

#include <Arduino.h>

struct JoystickKeyboardConfig {
  uint8_t x_pin;
  uint8_t y_pin;
  uint16_t center_x;
  uint16_t center_y;
  uint16_t low_threshold;
  uint16_t high_threshold;
  uint16_t deadzone_radius;
  bool invert_y;
};

class JoystickKeyboard {
 public:
  void setup(const JoystickKeyboardConfig& config);
  void process();
};
