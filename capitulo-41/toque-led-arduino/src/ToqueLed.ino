const int TOQUE = 4;                // T0 no ESP32, T4 no S3
const int LED = 5;
uint32_t referencia = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  uint64_t soma = 0;
  for (int i = 0; i < 50; i++) {    // 1 s sem tocar: a referência
    soma += touchRead(TOQUE);
    delay(20);
  }
  referencia = soma / 50;
  Serial.printf("referência: %lu\n", (unsigned long)referencia);
}

void loop() {
  uint32_t v = touchRead(TOQUE);
#if CONFIG_IDF_TARGET_ESP32
  bool tocado = v < referencia * 97 / 100;   // versão 1: o toque baixa a leitura
#else
  bool tocado = v > referencia * 103 / 100;  // versão 2 (S2, S3): o toque sobe a leitura
#endif
  digitalWrite(LED, tocado);
  Serial.printf("%lu %s\n", (unsigned long)v, tocado ? "TOQUE" : "");
  delay(50);
}
