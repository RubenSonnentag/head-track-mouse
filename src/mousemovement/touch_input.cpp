#include "mousemovement/touch_input.h"

#include <Arduino.h>

#include "mousemovement/head_track_config.h"
#include "mousemovement/runtime_config.h"

namespace {

uint16_t baseline = 0;
uint16_t last_raw = 0;
bool touched = false;
uint8_t activate_streak = 0;
uint8_t release_streak = 0;

uint16_t sample_touch_raw() {
  const auto& pins = mousemovement_runtime::config().pins;
  digitalWriteFast(pins.touch_send, LOW);
  pinMode(pins.touch_receive, OUTPUT);
  digitalWriteFast(pins.touch_receive, LOW);
  delayMicroseconds(5);

  pinMode(pins.touch_receive, INPUT);
  digitalWriteFast(pins.touch_send, HIGH);

  uint16_t count = 0;
  while (digitalReadFast(pins.touch_receive) == LOW && count < head_track_config::TOUCH_SAMPLE_TIMEOUT) {
    ++count;
  }

  digitalWriteFast(pins.touch_send, LOW);
  return count;
}

void update_baseline(uint16_t raw) {
  const float blend = head_track_config::TOUCH_BASELINE_BLEND;
  baseline = static_cast<uint16_t>((baseline * (1.0f - blend)) + (raw * blend));
}

uint16_t capture_idle_baseline() {
  for (uint16_t i = 0; i < head_track_config::TOUCH_WARMUP_SAMPLES; ++i) {
    sample_touch_raw();
    delayMicroseconds(50);
  }

  uint32_t sum = 0;
  for (uint16_t i = 0; i < head_track_config::TOUCH_BASELINE_SAMPLES; ++i) {
    sum += sample_touch_raw();
    delayMicroseconds(50);
  }

  return static_cast<uint16_t>(sum / head_track_config::TOUCH_BASELINE_SAMPLES);
}

}  // namespace

void touch_input_begin() {
  const auto& pins = mousemovement_runtime::config().pins;
  pinMode(pins.touch_send, OUTPUT);
  digitalWriteFast(pins.touch_send, LOW);
  pinMode(pins.touch_receive, INPUT);

  baseline = capture_idle_baseline();
  last_raw = baseline;
  touched = false;
  activate_streak = 0;
  release_streak = 0;
}

bool touch_input_is_active() {
  last_raw = sample_touch_raw();

  if (!touched) {
    update_baseline(last_raw);
    if (last_raw > (baseline + head_track_config::TOUCH_THRESHOLD_OFFSET)) {
      if (activate_streak < head_track_config::TOUCH_ACTIVATE_STABLE_SAMPLES) {
        ++activate_streak;
      }
    } else {
      activate_streak = 0;
    }

    if (activate_streak >= head_track_config::TOUCH_ACTIVATE_STABLE_SAMPLES) {
      touched = true;
      activate_streak = 0;
      release_streak = 0;
    }
  } else {
    if (last_raw > (baseline + head_track_config::TOUCH_RELEASE_OFFSET)) {
      release_streak = 0;
    } else {
      if (release_streak < head_track_config::TOUCH_RELEASE_STABLE_SAMPLES) {
        ++release_streak;
      }
    }

    if (release_streak >= head_track_config::TOUCH_RELEASE_STABLE_SAMPLES) {
      touched = false;
      release_streak = 0;
      activate_streak = 0;
      update_baseline(last_raw);
    }
  }

  return touched;
}

uint16_t touch_input_last_raw() {
  return last_raw;
}

uint16_t touch_input_baseline() {
  return baseline;
}
