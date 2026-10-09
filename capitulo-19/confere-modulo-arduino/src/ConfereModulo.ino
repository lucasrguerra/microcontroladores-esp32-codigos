#include "esp_flash.h"

// Copie aqui o part number gravado na blindagem do módulo
const char *PART_NUMBER = "ESP32-S3-WROOM-1-N16R8";

void setup() {
  Serial.begin(115200);
  delay(500);

  // O campo de memória começa no último "-N" ou "-H" seguido de número
  const char *campo = nullptr;
  for (const char *p = PART_NUMBER; *p; p++) {
    if (p[0] == '-' && (p[1] == 'N' || p[1] == 'H') && isdigit(p[2])) {
      campo = p + 1;
    }
  }
  if (campo == nullptr) {
    Serial.println("Part number sem campo de memória");
    return;
  }
  char *fim;
  long flashEsperada = strtol(campo + 1, &fim, 10);
  long psramEsperada = (*fim == 'R') ? strtol(fim + 1, nullptr, 10) : 0;

  uint32_t fisica = 0;
  esp_flash_get_physical_size(nullptr, &fisica);
  long flashMB = fisica / (1024 * 1024);
  long psramMB = (ESP.getPsramSize() + 512 * 1024) / (1024 * 1024);

  Serial.printf("%s, revisão %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("Flash: esperada %ld MB, encontrada %ld MB\n", flashEsperada, flashMB);
  Serial.printf("PSRAM: esperada %ld MB, encontrada %ld MB\n", psramEsperada, psramMB);
  bool ok = (flashMB == flashEsperada) && (psramMB == psramEsperada);
  Serial.println(ok ? "Placa conforme o part number" : "Placa DIVERGENTE do part number");
}

void loop() {}
