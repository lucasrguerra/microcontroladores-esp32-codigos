RTC_DATA_ATTR int despertares = 0;   // fica na memória RTC: sobrevive ao deep sleep
const int LED_RGB = 8;               // LED endereçável da ESP32-C3-DevKitM-1

void setup() {
  Serial.begin(115200);
  delay(1000);                       // tempo para o monitor serial reconectar

  despertares++;
  Serial.printf("Despertar número %d\n", despertares);

  rgbLedWrite(LED_RGB, 0, 0, 32);    // azul fraco por meio segundo
  delay(500);
  rgbLedWrite(LED_RGB, 0, 0, 0);

  esp_sleep_enable_timer_wakeup(10ULL * 1000000);  // acorda em 10 s
  esp_deep_sleep_start();
}

void loop() {}  // nunca chega aqui: o chip reinicia a cada despertar
