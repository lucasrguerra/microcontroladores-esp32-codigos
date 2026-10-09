#include <Wire.h>

const int PINO_SDA = 4;
const int PINO_SCL = 5;

void setup() {
  Serial.begin(115200);
  Wire.begin(PINO_SDA, PINO_SCL, 100000);   // 100 kHz: o mais tolerante
  Wire.setTimeOut(50);                      // ms por transação
}

void loop() {
  int achados = 0;
  Serial.println("varrendo 0x08 a 0x77...");
  for (uint8_t end = 0x08; end <= 0x77; end++) {
    Wire.beginTransmission(end);
    uint8_t r = Wire.endTransmission();     // 0 = alguém respondeu com ACK
    if (r == 0) {
      Serial.printf("  dispositivo em 0x%02X\n", end);
      achados++;
    } else if (r == 5) {
      Serial.println("  timeout: SCL preso em baixo? confira os pull-ups");
      break;
    }
  }
  Serial.printf("%d dispositivo(s)\n\n", achados);
  delay(5000);
}
