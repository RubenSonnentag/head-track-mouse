#include "mouseclick/MouseClick.h"

#include <usb_mouse.h>

#include "logging.h"

namespace {

enum class SipPuffState : uint8_t {
  Neutral,
  Sip,
  Puff,
};

MouseClickConfig config{};
SipPuffState state = SipPuffState::Neutral;

void apply_mouse_buttons(SipPuffState next_state) {
  switch (next_state) {
    case SipPuffState::Neutral:
      Mouse.release(MOUSE_LEFT);
      Mouse.release(MOUSE_RIGHT);
      break;
    case SipPuffState::Sip:
      Mouse.release(MOUSE_LEFT);
      Mouse.press(MOUSE_RIGHT);
      break;
    case SipPuffState::Puff:
      Mouse.release(MOUSE_RIGHT);
      Mouse.press(MOUSE_LEFT);
      break;
  }
}

SipPuffState detect_state(uint16_t raw) {
  if (state == SipPuffState::Sip) {
    return raw <= config.neutral_max ? SipPuffState::Neutral : SipPuffState::Sip;
  }

  if (state == SipPuffState::Puff) {
    return raw >= config.neutral_min ? SipPuffState::Neutral : SipPuffState::Puff;
  }

  if (raw >= config.sip_threshold) {
    return SipPuffState::Sip;
  }

  if (raw <= config.puff_threshold) {
    return SipPuffState::Puff;
  }

  return SipPuffState::Neutral;
}

}  // namespace

void MouseClick::setup(const MouseClickConfig& new_config) {
  config = new_config;
  pinMode(config.input_pin, INPUT);
  state = SipPuffState::Neutral;
  apply_mouse_buttons(state);
  LOG_MOUSE_CLICK("setup pin=%u sip>=%u puff<=%u neutral=%u..%u", config.input_pin, config.sip_threshold,
                  config.puff_threshold, config.neutral_min, config.neutral_max);
}

void MouseClick::process() {
  const uint16_t raw = analogRead(config.input_pin);
  const SipPuffState next_state = detect_state(raw);
  if (next_state == state) {
    return;
  }

  state = next_state;
  apply_mouse_buttons(state);
#if LOG_ENABLE_SIP_PUFF
  const char* state_name = "neutral";
  if (state == SipPuffState::Sip) {
    state_name = "sip";
  } else if (state == SipPuffState::Puff) {
    state_name = "puff";
  }
  LOG_MOUSE_CLICK("state=%s raw=%u", state_name, raw);
#endif
}
