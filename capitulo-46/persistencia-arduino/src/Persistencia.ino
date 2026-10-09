#include <Preferences.h>
#include <LittleFS.h>

Preferences prefs;

void setup() {
  Serial.begin(115200);
  delay(1000);

  prefs.begin("app", false);              // namespace "app", leitura e escrita
  uint32_t boots = prefs.getUInt("boots", 0) + 1;
  prefs.putUInt("boots", boots);
  prefs.end();
  Serial.printf("boot número %lu\n", (unsigned long)boots);

  if (!LittleFS.begin(true)) {            // true: formata se não houver sistema
    Serial.println("LittleFS indisponível: confira a tabela de partições");
    return;
  }
  File f = LittleFS.open("/registro.txt", FILE_APPEND);
  f.printf("boot %lu em %lu ms\n", (unsigned long)boots, millis());
  f.close();

  f = LittleFS.open("/registro.txt", FILE_READ);
  Serial.printf("registro.txt: %u bytes\n", (unsigned)f.size());
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.printf("LittleFS: %u de %u bytes usados\n",
                (unsigned)LittleFS.usedBytes(), (unsigned)LittleFS.totalBytes());
}

void loop() {}
