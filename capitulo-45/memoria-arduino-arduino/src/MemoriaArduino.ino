// Mede o heap e usa a PSRAM quando a placa tem.
void mostrar(const char *quando) {
  Serial.printf("%-16s livre %7lu  maior bloco %7lu  mínimo %7lu\n", quando,
                (unsigned long)ESP.getFreeHeap(),
                (unsigned long)ESP.getMaxAllocHeap(),
                (unsigned long)ESP.getMinFreeHeap());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  mostrar("início");

  const size_t TAM = 200 * 1024;     // um quadro de câmera, um buffer de áudio
  uint8_t *buf;
  if (psramFound()) {
    Serial.printf("PSRAM: %lu bytes, %lu livres\n",
                  (unsigned long)ESP.getPsramSize(),
                  (unsigned long)ESP.getFreePsram());
    buf = (uint8_t *)ps_malloc(TAM);
  } else {
    buf = (uint8_t *)malloc(TAM);
  }
  if (buf == nullptr) {
    Serial.println("sem um bloco de 200 KB: use uma placa com PSRAM");
    return;
  }
  memset(buf, 0, TAM);
  mostrar("com o buffer");
  free(buf);
  mostrar("depois do free");
}

void loop() {}
