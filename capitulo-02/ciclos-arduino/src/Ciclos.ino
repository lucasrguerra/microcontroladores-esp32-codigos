void setup() {
  Serial.begin(115200);
  delay(500);

  volatile uint32_t soma = 0;          // volatile: o compilador não pode eliminar o laço
  uint32_t t0 = ESP.getCycleCount();
  for (int i = 0; i < 1000; i++) {
    soma += i;
  }
  uint32_t t1 = ESP.getCycleCount();

  Serial.printf("CPU a %lu MHz\n", (unsigned long)getCpuFrequencyMhz());
  Serial.printf("1000 somas: %lu ciclos (%.1f ciclos por volta)\n",
                (unsigned long)(t1 - t0), (t1 - t0) / 1000.0);
}

void loop() {}
