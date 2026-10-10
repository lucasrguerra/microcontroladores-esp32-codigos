// Linha de base: a mesma soma dos Caps. 75 e 76, mais a memória livre.
static uint32_t soma(uint32_t n) {
  volatile uint32_t limite = n;
  uint32_t s = 0;
  for (uint32_t i = 0; i < limite; i++) s += i & 0xFF;
  return s;
}
void setup() {
  Serial.begin(115200);
  uint32_t t0 = micros();
  uint32_t r = soma(100000);
  Serial.printf("soma: %u em %u us; heap livre %u\n", r, micros() - t0, ESP.getFreeHeap());
}
void loop() {}
