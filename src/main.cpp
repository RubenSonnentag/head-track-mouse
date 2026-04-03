#include <Arduino.h>

#include "gyro.h"
#include "head_track_config.h"
#include "imu.h"
#include "logging.h"
#include "mouse_pipeline.h"

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
elapsedMillis heartbeat_timer;

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
  log_calibration_step("Start per A2. Bitte ruhig liegen lassen.");
  if (!imu_calibrate()) {
    log_error(F("Kalibrierung fehlgeschlagen."));
    mouse_pipeline_reset();
    run_state = RunState::Paused;
    log_state_change("paused");
    return;
  }

  mouse_pipeline_reset();
  gyro_reset();
  run_state = RunState::Active;
  log_state_change("active");
}

void process_button_action() {
  if (!handle_button_press()) {
    return;
  }

  log_button_pressed(run_state == RunState::Active ? "pause requested" : "calibration requested");
  if (run_state == RunState::Active) {
    mouse_pipeline_reset();
    run_state = RunState::Paused;
    log_state_change("paused");
  } else {
    activate_after_calibration();
  }
}

void emit_heartbeat() {
  if (heartbeat_timer < 1000) {
    return;
  }

  heartbeat_timer = 0;
  log_infof("heartbeat state=%s imu_ready=%s calibration=%s button=%s", run_state == RunState::Active ? "active" : "paused",
            imu_is_ready() ? "true" : "false", imu_has_calibration() ? "true" : "false",
            digitalRead(head_track_config::PIN_BUTTON_A2) == LOW ? "pressed" : "released");
}

}  // namespace

void setup() {
  Serial.begin(head_track_config::SERIAL_BAUD);
  while (!Serial && millis() < 4000) {
    delay(10);
  }
  log_begin();

  pinMode(head_track_config::PIN_BUTTON_A2, INPUT_PULLUP);
  mouse_pipeline_begin();
  gyro_update_sensitivity();
  gyro_set_logging_enabled(true);
  mouse_pipeline_reset();
  gyro_reset();

  log_info(F("head-track-mouse bootet."));
  if (imu_init()) {
    log_info(F("Beide IMUs initialisiert."));
  } else {
    log_error(F("IMU-Initialisierung unvollstaendig. Bitte Verkabelung pruefen."));
  }
  imu_print_status();
  log_info(F("A2 druecken: kalibrieren und aktivieren. Danach erneut druecken: pausieren."));
  log_state_change("paused");

  loop_timer = 0;
  heartbeat_timer = 0;
}

void loop() {
  process_button_action();
  emit_heartbeat();

  if (run_state == RunState::Active && imu_has_calibration() && loop_timer >= head_track_config::LOOP_INTERVAL_US) {
    loop_timer = 0;
    gyro_report_incremental(true);
    mouse_pipeline_flush();
  }
}
