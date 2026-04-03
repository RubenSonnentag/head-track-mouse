// Minimales Hello-World-Beispiel fuer einen Teensy 4.1.
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    delay(10);
  }

  Serial.println("Hello, world!");
}

void loop() {
  delay(1000);
}
