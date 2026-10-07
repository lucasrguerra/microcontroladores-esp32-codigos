void setup() {
  Serial.begin(115200);
  delay(500);

  uint16_t rev = ESP.getChipRevision();   // maior * 100 + menor: 301 = v3.1
  Serial.printf("%s revisão v%u.%u, %u núcleo(s)\n", ESP.getChipModel(),
                rev / 100, rev % 100, ESP.getChipCores());
  Serial.printf("PSRAM: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Flash: %u bytes\n", ESP.getFlashChipSize());

  if (rev < 300) {
    Serial.println("Revisão anterior à v3.0: PSRAM com contorno e sem Secure Boot V2.");
  }
}

void loop() {}
