#include <Arduino.h>
#include <Mouse.h>

#include "gyro.h"
#include "head_track_config.h"
#include "imu.h"

namespace {

enum class RunState : uint8_t {
  Paused,
  Active,
};

RunState run_state = RunState::Paused;
bool button_state = HIGH;
bool last_button_reading = HIGH;
uint32_t last_debounce_time = 0;
elapsedMicros loop_timer;

bool handle_button_press() {
  const bool reading = digitalRead(head_track_config::PIN_BUTTON_A2);
  if (reading != last_button_reading) {
    last_debounce_time = millis();
    last_button_reading = reading;
  }

  if ((millis() - last_debounce_time) > head_track_config::BUTTON_DEBOUNCE_MS && reading != button_state) {
    button_state = reading;
    if (button_state == LOW) {
      return true;
    }
  }

  return false;
}

void activate_after_calibration() {
  Serial.println("Kalibrierung startet. Bitte ruhig liegen lassen.");
  if (!imu_calibrate()) {
    Serial.println("Kalibrierung fehlgeschlagen.");
    run_state = RunState::Paused;
    return;
  }

  gyro_reset();
  run_state = RunState::Active;
  Serial.println("Kalibrierung abgeschlossen. Maussteuerung aktiv.");
}

void process_button_action() {
  if (!handle_button_press()) {
    return;
  }

  if (run_state == RunState::Active) {
    run_state = RunState::Paused;
    Serial.println("Maussteuerung pausiert.");
  } else {
    activate_after_calibration();
  }
}

}  // namespace

void setup() {
  Serial.begin(head_track_config::SERIAL_BAUD);
  while (!Serial && millis() < 4000) {
    delay(10);
  }

  pinMode(head_track_config::PIN_BUTTON_A2, INPUT_PULLUP);
  Mouse.begin();
  gyro_update_sensitivity();
  gyro_reset();

  Serial.println("head-track-mouse bootet.");
  if (imu_init()) {
    Serial.println("Beide IMUs initialisiert.");
  } else {
    Serial.println("IMU-Initialisierung unvollstaendig. Bitte Verkabelung pruefen.");
  }
  imu_print_status();
  Serial.println("A2 druecken: kalibrieren und aktivieren. Danach erneut druecken: pausieren.");

  loop_timer = 0;
}

void loop() {
  process_button_action();

  if (run_state == RunState::Active && imu_has_calibration() && loop_timer >= head_track_config::LOOP_INTERVAL_US) {
    loop_timer = 0;
    gyro_report_incremental(true);
  }
}
