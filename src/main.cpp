#include <Arduino.h>

#include "joystick/JoystickKeyboard.h"
#include "logging.h"
#include "mousemovement/MouseMovement.h"
#include "sippuff/SipPuffClicks.h"

namespace {

MouseMovement mouseMovement;
SipPuffClicks sipPuffClicks;
JoystickKeyboard joystickKeyboard;

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
          },
  };

  mouseMovement.begin(config);

  SipPuffClicksConfig sip_puff_config{
      .input_pin = A0,
      .sip_threshold = 640,
      .puff_threshold = 384,
      .neutral_min = 480,
      .neutral_max = 544,
  };
  sipPuffClicks.setup(sip_puff_config);

  JoystickKeyboardConfig joystick_config{
      .x_pin = A1,
      .y_pin = A2,
      .center_x = 512,
      .center_y = 512,
      .low_threshold = 384,
      .high_threshold = 640,
      .deadzone_radius = 64,
      .invert_y = false,
  };
  joystickKeyboard.setup(joystick_config);
}

void loop() {
  mouseMovement.process();
  sipPuffClicks.process();
  joystickKeyboard.process();
}
