#pragma once

#include <Arduino.h>

struct MouseClickConfig {
  uint8_t input_pin;
  uint16_t sip_threshold;
  uint16_t puff_threshold;
  uint16_t neutral_min;
  uint16_t neutral_max;
};

class MouseClick {
 public:
  void setup(const MouseClickConfig& config);
  void process();
};
