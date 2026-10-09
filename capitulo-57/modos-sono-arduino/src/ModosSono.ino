// Light sleep e deep sleep lado a lado: o que volta e o que se perde.
#include "esp_sleep.h"

RTC_DATA_ATTR int acordadas = 0;     // memória RTC: sobrevive ao deep sleep
int naRam = 0;                       // SRAM comum: zera a cada deep sleep

void setup() {
  Serial.begin(115200);
  delay(500);
  naRam++;
  Serial.printf("início: acordadas=%d, naRam=%d, causa=%d\n",
                acordadas, naRam, (int)esp_sleep_get_wakeup_cause());

  // 1) light sleep: o programa continua na linha seguinte.
  esp_sleep_enable_timer_wakeup(2 * 1000000ULL);
  Serial.println("light sleep por 2 s");
  Serial.flush();                    // esvazia a UART antes de dormir
  uint32_t t0 = millis();
  esp_light_sleep_start();
  Serial.printf("voltou após %lu ms, naRam=%d\n", millis() - t0, naRam);

  // 2) deep sleep: o chip reinicia e o setup() roda de novo.
  acordadas++;
  esp_sleep_enable_timer_wakeup(5 * 1000000ULL);
  Serial.println("deep sleep por 5 s");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {}
