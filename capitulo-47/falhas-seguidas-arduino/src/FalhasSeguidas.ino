#include "esp_system.h"

RTC_NOINIT_ATTR uint32_t falhas_seguidas;   // sobrevive ao reset

const char *nome_reset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "energia ligada";
    case ESP_RST_SW:        return "esp_restart()";
    case ESP_RST_PANIC:     return "pane";
    case ESP_RST_INT_WDT:   return "watchdog de interrupção";
    case ESP_RST_TASK_WDT:  return "watchdog de tarefas";
    case ESP_RST_WDT:       return "outro watchdog";
    case ESP_RST_BROWNOUT:  return "queda de tensão";
    case ESP_RST_DEEPSLEEP: return "saída do deep sleep";
    default:                return "outro";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  esp_reset_reason_t r = esp_reset_reason();
  bool falha = r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT ||
               r == ESP_RST_WDT || r == ESP_RST_BROWNOUT;
  if (r == ESP_RST_POWERON || !falha) {
    falhas_seguidas = 0;
  } else {
    falhas_seguidas++;
  }
  Serial.printf("último reset: %s; falhas seguidas: %lu\n",
                nome_reset(r), (unsigned long)falhas_seguidas);
  if (falhas_seguidas >= 3) {
    log_e("3 falhas seguidas: modo seguro");      // atuadores desligados, só OTA
  }
  Serial.println("envie 'p' para provocar uma pane");
}

void loop() {
  if (Serial.read() == 'p') {
    abort();
  }
}
