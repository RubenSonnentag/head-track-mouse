#pragma once

#include <stdint.h>

void touch_input_begin();
bool touch_input_is_active();
uint16_t touch_input_last_raw();
uint16_t touch_input_baseline();
