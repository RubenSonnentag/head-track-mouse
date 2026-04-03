#include "mousemovement/MouseMovement.h"

#include "mousemovement/imu.h"
#include "logging.h"
#include "mousemovement/mouse_pipeline.h"
#include "mousemovement/internal_gyro.h"
#include "mousemovement/runtime_config.h"
#include "mousemovement/touch_input.h"

namespace {

enum class RunState : uint8_t {
  Ready,
  Calibrating,
};

RunState run_state = RunState::Ready;
bool button_state = HIGH;
bool last_button_reading = HIGH;
uint32_t last_debounce_time = 0;
elapsedMicros loop_timer;
elapsedMillis heartbeat_timer;
bool touch_active = false;
bool last_touch_active = false;

const MouseMovementConfig& config() {
  return mousemovement_runtime::config();
}

bool handle_button_press() {
  const bool reading = digitalRead(config().pins.calibrate_button);
  if (reading != last_button_reading) {
    last_debounce_time = millis();
    last_button_reading = reading;
  }

  if ((millis() - last_debounce_time) > config().settings.button_debounce_ms && reading != button_state) {
    button_state = reading;
    if (button_state == LOW) {
      return true;
    }
  }

  return false;
}

void activate_after_calibration() {
  LOG_MM_CORE("Kalibrierung: Start per A2. Bitte ruhig liegen lassen.");
  run_state = RunState::Calibrating;
  LOG_MM_CORE("State -> calibrating");
  if (!imu_calibrate()) {
    LOG_MM_CORE("Kalibrierung fehlgeschlagen.");
    mouse_pipeline_reset();
    run_state = RunState::Ready;
    LOG_MM_CORE("State -> ready");
    return;
  }

  mouse_pipeline_reset();
  gyro_reset();
  run_state = RunState::Ready;
  LOG_MM_CORE("State -> ready");
}

void process_button_action() {
  if (!handle_button_press()) {
    return;
  }

  LOG_MM_CORE("Button gedrueckt (calibration requested)");
  activate_after_calibration();
}

void emit_heartbeat() {
  if (heartbeat_timer < 1000) {
    return;
  }

  heartbeat_timer = 0;
  const char* state = run_state == RunState::Calibrating ? "calibrating" : "ready";
  (void)state;
  LOG_MM_CORE("heartbeat state=%s imu_ready=%s calibration=%s button=%s touch=%s raw=%u baseline=%u", state,
              imu_is_ready() ? "true" : "false", imu_has_calibration() ? "true" : "false",
              digitalRead(config().pins.calibrate_button) == LOW ? "pressed" : "released",
              touch_active ? "active" : "idle", touch_input_last_raw(), touch_input_baseline());
}

void update_touch_state() {
  touch_active = touch_input_is_active();
  if (touch_active == last_touch_active) {
    return;
  }

  last_touch_active = touch_active;
  mouse_pipeline_reset();
  if (!touch_active) {
    LOG_MM_TOUCH("Touch aus: Maus angehoben. raw=%u baseline=%u delta=%d", touch_input_last_raw(),
                 touch_input_baseline(),
                 static_cast<int>(touch_input_last_raw()) - static_cast<int>(touch_input_baseline()));
  } else {
    LOG_MM_TOUCH("Touch an: Maus wieder aufgesetzt. raw=%u baseline=%u delta=%d", touch_input_last_raw(),
                 touch_input_baseline(),
                 static_cast<int>(touch_input_last_raw()) - static_cast<int>(touch_input_baseline()));
  }
}

}  // namespace

void MouseMovement::begin(const MouseMovementConfig& new_config) {
  mousemovement_runtime::set_config(new_config);

  pinMode(config().pins.calibrate_button, INPUT_PULLUP);
  touch_input_begin();
  mouse_pipeline_begin();
  gyro_update_sensitivity(config().settings.sensitivity_multiplier);
  mouse_pipeline_reset();
  gyro_reset();

  LOG_MM_CORE("head-track-mouse bootet.");
  if (imu_init()) {
    LOG_MM_CORE("Beide IMUs initialisiert.");
  } else {
    LOG_MM_CORE("IMU-Initialisierung unvollstaendig. Bitte Verkabelung pruefen.");
  }
  imu_print_status();
  LOG_MM_CORE("A2 druecken: Kalibrierung starten. Touch steuert Maus an/aus.");
  LOG_MM_CORE("State -> ready");

  loop_timer = 0;
  heartbeat_timer = 0;
}

void MouseMovement::process() {
  process_button_action();
  update_touch_state();
  static elapsedMillis elapsed_since_touch_log;
  if (elapsed_since_touch_log >= 250) {
    elapsed_since_touch_log = 0;
    LOG_MM_TOUCH("active=%s raw=%u baseline=%u delta=%d", touch_active ? "true" : "false", touch_input_last_raw(),
                 touch_input_baseline(), static_cast<int>(touch_input_last_raw()) - static_cast<int>(touch_input_baseline()));
  }
  emit_heartbeat();

  if (run_state == RunState::Ready && imu_has_calibration() && loop_timer >= config().settings.loop_interval_us) {
    loop_timer = 0;
    gyro_report_incremental(touch_active);
    if (touch_active) {
      mouse_pipeline_flush();
    }
  }
}
