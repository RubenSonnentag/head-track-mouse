#pragma once

#include <Arduino.h>

#include "mousemovement/alpakka_math.h"
#include "logging_config.h"

void log_begin();
void log_write(const char* component, const char* format, ...);

#if LOG_ENABLE_MOUSEMOVEMENT_CORE
#define LOG_MM_CORE(fmt, ...) log_write("mousemovement.core", fmt, ##__VA_ARGS__)
#else
#define LOG_MM_CORE(fmt, ...) ((void)0)
#endif

#if LOG_ENABLE_MOUSEMOVEMENT_IMU
#define LOG_MM_IMU(fmt, ...) log_write("mousemovement.imu", fmt, ##__VA_ARGS__)
#else
#define LOG_MM_IMU(fmt, ...) ((void)0)
#endif

#if LOG_ENABLE_MOUSEMOVEMENT_TOUCH
#define LOG_MM_TOUCH(fmt, ...) log_write("mousemovement.touch", fmt, ##__VA_ARGS__)
#else
#define LOG_MM_TOUCH(fmt, ...) ((void)0)
#endif

#if LOG_ENABLE_MOUSEMOVEMENT_TELEMETRY
#define LOG_MM_TELEMETRY(fmt, ...) log_write("mousemovement.telemetry", fmt, ##__VA_ARGS__)
#else
#define LOG_MM_TELEMETRY(fmt, ...) ((void)0)
#endif
