#pragma once

#include <Arduino.h>

void gyro_reset();
void gyro_update_sensitivity(double multiplier = 1.0);
void gyro_report_incremental(bool emit_mouse);
void gyro_set_logging_enabled(bool enabled);
