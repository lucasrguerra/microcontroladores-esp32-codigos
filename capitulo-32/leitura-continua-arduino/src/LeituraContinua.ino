// Dois pinos do ADC1 (o modo contínuo do Arduino só aceita o ADC1)
#if CONFIG_IDF_TARGET_ESP32
const uint8_t PINOS[] = {34, 35};
#elif CONFIG_IDF_TARGET_ESP32C3
const uint8_t PINOS[] = {3, 4};
#else
const uint8_t PINOS[] = {4, 5};     // S3 e C6
#endif
const size_t N = sizeof(PINOS);

volatile bool pronto = false;
adc_continuous_result_t *resultado = nullptr;

void ARDUINO_ISR_ATTR quadro_pronto() {
  pronto = true;
}

void setup() {
  Serial.begin(115200);
  analogContinuousSetWidth(12);
  analogContinuousSetAtten(ADC_11db);
  // 1000 conversões por pino em cada quadro, 20 000 conversões por segundo no total
  if (!analogContinuous(PINOS, N, 1000, 20000, &quadro_pronto)) {
    Serial.println("falha ao configurar o modo contínuo");
    while (true) delay(1000);
  }
  analogContinuousStart();
}

void loop() {
  if (pronto) {
    pronto = false;
    if (analogContinuousRead(&resultado, 0)) {
      for (size_t i = 0; i < N; i++) {
        Serial.printf("GPIO %u: %4d mV   ", resultado[i].pin,
                      resultado[i].avg_read_mvolts);
      }
      Serial.println();
    }
  }
}
