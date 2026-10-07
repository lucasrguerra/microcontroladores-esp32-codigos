const int LARGURA = 480;
const int ALTURA = 320;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.printf("SRAM interna livre: %u bytes\n", ESP.getFreeHeap());
  if (!psramFound()) {
    Serial.println("PSRAM não encontrada: confira Tools > PSRAM no menu da placa.");
    return;
  }
  Serial.printf("PSRAM: %u bytes, livre: %u\n", ESP.getPsramSize(), ESP.getFreePsram());

  // Quadro de 480x320 em RGB565: 2 bytes por pixel, 307 200 bytes
  size_t tamanho = LARGURA * ALTURA * sizeof(uint16_t);
  uint16_t *quadro = (uint16_t *)ps_malloc(tamanho);
  if (quadro == NULL) {
    Serial.println("Sem memória para o quadro.");
    return;
  }
  for (int i = 0; i < LARGURA * ALTURA; i++) {
    quadro[i] = 0xF800;  // vermelho em RGB565
  }
  Serial.printf("Quadro de %u bytes pronto na PSRAM.\n", tamanho);
}

void loop() {}
