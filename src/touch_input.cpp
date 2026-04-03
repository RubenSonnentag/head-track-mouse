#include "touch_input.h"

#include <Arduino.h>

#include "head_track_config.h"

namespace {

uint16_t baseline = 0;
uint16_t last_raw = 0;
bool touched = false;

uint16_t sample_touch_raw() {
  digitalWriteFast(head_track_config::PIN_TOUCH_SEND, LOW);
  pinMode(head_track_config::PIN_TOUCH_RECEIVE, OUTPUT);
  digitalWriteFast(head_track_config::PIN_TOUCH_RECEIVE, LOW);
  delayMicroseconds(5);

  pinMode(head_track_config::PIN_TOUCH_RECEIVE, INPUT);
  digitalWriteFast(head_track_config::PIN_TOUCH_SEND, HIGH);

  uint16_t count = 0;
  while (digitalReadFast(head_track_config::PIN_TOUCH_RECEIVE) == LOW &&
         count < head_track_config::TOUCH_SAMPLE_TIMEOUT) {
    ++count;
  }

  digitalWriteFast(head_track_config::PIN_TOUCH_SEND, LOW);
  return count;
}

void update_baseline(uint16_t raw) {
  const float blend = head_track_config::TOUCH_BASELINE_BLEND;
  baseline = static_cast<uint16_t>((baseline * (1.0f - blend)) + (raw * blend));
}

}  // namespace

void touch_input_begin() {
  pinMode(head_track_config::PIN_TOUCH_SEND, OUTPUT);
  digitalWriteFast(head_track_config::PIN_TOUCH_SEND, LOW);
  pinMode(head_track_config::PIN_TOUCH_RECEIVE, INPUT);

  uint32_t sum = 0;
  for (uint16_t i = 0; i < head_track_config::TOUCH_BASELINE_SAMPLES; ++i) {
    sum += sample_touch_raw();
    delayMicroseconds(50);
  }

  baseline = static_cast<uint16_t>(sum / head_track_config::TOUCH_BASELINE_SAMPLES);
  last_raw = baseline;
  touched = false;
}

bool touch_input_is_active() {
  last_raw = sample_touch_raw();

  if (!touched) {
    update_baseline(last_raw);
    touched = last_raw > (baseline + head_track_config::TOUCH_THRESHOLD_OFFSET);
  } else {
    touched = last_raw > (baseline + head_track_config::TOUCH_RELEASE_OFFSET);
    if (!touched) {
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
