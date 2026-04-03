#pragma once

#include <Arduino.h>

struct SipPuffClicksConfig {
  uint8_t input_pin;
  uint16_t sip_threshold;
  uint16_t puff_threshold;
  uint16_t neutral_min;
  uint16_t neutral_max;
};

class SipPuffClicks {
 public:
  void setup(const SipPuffClicksConfig& config);
  void process();
};
