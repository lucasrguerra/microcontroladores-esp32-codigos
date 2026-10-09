// Pino com ADC1: GPIO36 no ESP32, GPIO1 no S3, GPIO0 no C3 e no C6
const int PINO_BATERIA = A0;
const float DIVISOR = 2.0;        // R1 = R2 = 100 kΩ
const int BATERIA_BAIXA_MV = 3400;

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);
}

void loop() {
  uint32_t soma = 0;
  for (int i = 0; i < 16; i++) {
    soma += analogReadMilliVolts(PINO_BATERIA);
  }
  int vbat = (soma / 16) * DIVISOR;
  Serial.printf("bateria: %d mV\n", vbat);
  if (vbat < BATERIA_BAIXA_MV) {
    Serial.println("bateria baixa: suspender gravações e transmissões");
  }
  delay(5000);
}
