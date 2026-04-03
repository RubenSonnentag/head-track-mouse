#include "mousemovement/mouse_pipeline.h"

#include <Arduino.h>

#include "mousemovement/alpakka_math.h"
#include "mousemovement/teensy_mouse16.h"

namespace {

double pending_x = 0.0;
double pending_y = 0.0;

int16_t quantize_axis(double value) {
  const long rounded = lround(value);
  return static_cast<int16_t>(clamp_value<long>(rounded, -32767, 32767));
}

}  // namespace

void mouse_pipeline_begin() {
  mouse_pipeline_reset();
}

void mouse_pipeline_reset() {
  pending_x = 0.0;
  pending_y = 0.0;
}

void mouse_pipeline_add_delta(double dx, double dy) {
  pending_x += dx;
  pending_y += dy;
}

void mouse_pipeline_flush() {
  const int16_t move_x = quantize_axis(pending_x);
  const int16_t move_y = quantize_axis(pending_y);

  if (move_x == 0 && move_y == 0) {
    return;
  }

  if (usb_mouse_move16(move_x, move_y, 0, 0) == 0) {
    pending_x -= static_cast<double>(move_x);
    pending_y -= static_cast<double>(move_y);
  }
}
