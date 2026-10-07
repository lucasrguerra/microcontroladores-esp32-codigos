void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.printf("Modelo:            %s\n", ESP.getChipModel());
  Serial.printf("Revisão de silício: %d\n", ESP.getChipRevision());
  Serial.printf("Núcleos:           %d\n", ESP.getChipCores());
  Serial.printf("CPU:               %lu MHz\n", (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash:             %lu MB\n", (unsigned long)(ESP.getFlashChipSize() / (1024 * 1024)));
  Serial.printf("PSRAM:             %lu KB\n", (unsigned long)(ESP.getPsramSize() / 1024));
  Serial.printf("Heap livre:        %lu KB\n", (unsigned long)(ESP.getFreeHeap() / 1024));
  Serial.printf("MAC (eFuse):       %012llX\n", ESP.getEfuseMac());
}

void loop() {}
