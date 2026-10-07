#include "esp_system.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.printf("Motivo do último reinício: %d\n", (int)esp_reset_reason());
}

void loop() {}
