#include <Arduino.h>

#include "logging.h"
#include "mousemovement/MouseMovement.h"

namespace {

MouseMovement mouseMovement;

}  // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    delay(10);
  }
  log_begin();

  MouseMovementConfig config{
      .pins =
          {
              .calibrate_button = 1,
              .touch_send = 2,
              .touch_receive = 3,
              .imu_cs0 = 10,
              .imu_cs1 = 9,
          },
      .settings =
          {
              .button_debounce_ms = 50,
              .loop_interval_us = 4000,
              .sensitivity_multiplier = 1.0,
              .logging_enabled = true,
          },
  };

  mouseMovement.begin(config);
}

void loop() {
  mouseMovement.process();
}
