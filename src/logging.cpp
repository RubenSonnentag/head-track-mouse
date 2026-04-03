#include "logging.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

void log_vprintf(const char* component, const char* format, va_list args) {
  char buffer[192];
  vsnprintf(buffer, sizeof(buffer), format, args);
  Serial.printf("[%10lu] [%s] ", millis(), component);
  Serial.print(buffer);
  Serial.print("\r\n");
}

}  // namespace

void log_begin() {
  log_write("logging", "Serial logging aktiv.");
}

void log_write(const char* component, const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_vprintf(component, format, args);
  va_end(args);
}
