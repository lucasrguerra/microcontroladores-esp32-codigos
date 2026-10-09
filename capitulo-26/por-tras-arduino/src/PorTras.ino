// Mostra o que o Arduino Core monta por trás do setup() e do loop().
#include "esp_idf_version.h"

void mostrar(const char *onde) {
  Serial.printf("[%s] tarefa \"%s\", núcleo %d, prioridade %u, "
                "pilha livre (mínimo) %u bytes\n",
                onde, pcTaskGetName(NULL), xPortGetCoreID(),
                (unsigned)uxTaskPriorityGet(NULL),
                (unsigned)uxTaskGetStackHighWaterMark(NULL));
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("Arduino Core %s sobre o ESP-IDF %s\n",
                ESP_ARDUINO_VERSION_STR, esp_get_idf_version());
  Serial.printf("Pilha do loopTask: %u bytes\n",
                (unsigned)getArduinoLoopTaskStackSize());
  mostrar("setup");
}

void loop() {
  mostrar("loop");
  delay(5000);
}
