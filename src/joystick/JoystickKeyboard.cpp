#include "joystick/JoystickKeyboard.h"

#include <usb_keyboard.h>
#include <keylayouts.h>

#include "logging.h"

namespace {

JoystickKeyboardConfig config{};
bool key_w_pressed = false;
bool key_a_pressed = false;
bool key_s_pressed = false;
bool key_d_pressed = false;

bool is_below_deadzone(uint16_t raw_value, uint16_t center_value) {
  return raw_value + config.deadzone_radius < center_value;
}

bool is_above_deadzone(uint16_t raw_value, uint16_t center_value) {
  return raw_value > center_value + config.deadzone_radius;
}

bool set_key_state(uint16_t keycode, bool desired_state, bool& current_state) {
  if (desired_state == current_state) {
    return false;
  }

  current_state = desired_state;
  if (desired_state) {
    Keyboard.press(keycode);
  } else {
    Keyboard.release(keycode);
  }

  return true;
}

void build_active_keys(char* buffer, size_t size) {
  size_t index = 0;
  if (key_w_pressed && index + 1 < size) {
    buffer[index++] = 'W';
  }
  if (key_a_pressed && index + 1 < size) {
    buffer[index++] = 'A';
  }
  if (key_s_pressed && index + 1 < size) {
    buffer[index++] = 'S';
  }
  if (key_d_pressed && index + 1 < size) {
    buffer[index++] = 'D';
  }

  if (index == 0 && size >= 5) {
    buffer[0] = 'n';
    buffer[1] = 'o';
    buffer[2] = 'n';
    buffer[3] = 'e';
    buffer[4] = '\0';
    return;
  }

  buffer[index] = '\0';
}

}  // namespace

void JoystickKeyboard::setup(const JoystickKeyboardConfig& new_config) {
  config = new_config;
  pinMode(config.x_pin, INPUT);
  pinMode(config.y_pin, INPUT);
  Keyboard.releaseAll();
  key_w_pressed = false;
  key_a_pressed = false;
  key_s_pressed = false;
  key_d_pressed = false;
  LOG_JOYSTICK_KEYBOARD("setup x=%u y=%u center=(%u,%u) low=%u high=%u deadzone=%u invert_y=%s", config.x_pin,
                        config.y_pin, config.center_x, config.center_y, config.low_threshold, config.high_threshold,
                        config.deadzone_radius, config.invert_y ? "true" : "false");
}

void JoystickKeyboard::process() {
  const uint16_t raw_x = analogRead(config.x_pin);
  const uint16_t raw_y = analogRead(config.y_pin);

  const bool physical_left = raw_y >= config.high_threshold && is_above_deadzone(raw_y, config.center_y);
  const bool physical_right = raw_y <= config.low_threshold && is_below_deadzone(raw_y, config.center_y);

  bool physical_up = raw_x >= config.high_threshold && is_above_deadzone(raw_x, config.center_x);
  bool physical_down = raw_x <= config.low_threshold && is_below_deadzone(raw_x, config.center_x);
  if (config.invert_y) {
    const bool previous_up = physical_up;
    physical_up = physical_down;
    physical_down = previous_up;
  }

  // Rotate the current directional assignment 90 degrees clockwise.
  const bool changed_a = set_key_state(KEY_A, physical_up, key_a_pressed);
  const bool changed_w = set_key_state(KEY_W, physical_left, key_w_pressed);
  const bool changed_d = set_key_state(KEY_D, physical_down, key_d_pressed);
  const bool changed_s = set_key_state(KEY_S, physical_right, key_s_pressed);

  if (changed_a || changed_d || changed_w || changed_s) {
    char active_keys[5];
    build_active_keys(active_keys, sizeof(active_keys));
    LOG_JOYSTICK_KEYBOARD("keys=%s raw_x=%u raw_y=%u", active_keys, raw_x, raw_y);
  }
}
