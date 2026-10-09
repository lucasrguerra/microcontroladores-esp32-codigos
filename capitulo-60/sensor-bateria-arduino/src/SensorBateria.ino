// Nó a bateria: acorda, mede a bateria, envia por ESP-NOW e volta a dormir.
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#if CONFIG_IDF_TARGET_ESP32
const int PINO_BATERIA = 34;
#elif CONFIG_IDF_TARGET_ESP32S3
const int PINO_BATERIA = 4;
#else
const int PINO_BATERIA = 2;
#endif
const int LIGA_DIVISOR = 5;            // MOSFET que liga o divisor 1:2
const uint64_t PERIODO_US = 300ULL * 1000000;
const uint8_t DIFUSAO[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

RTC_DATA_ATTR uint32_t envios = 0;

struct __attribute__((packed)) Leitura {
  uint32_t seq;
  uint16_t bateriaMv;
};

void setup() {
  gpio_hold_dis((gpio_num_t)LIGA_DIVISOR);
  pinMode(LIGA_DIVISOR, OUTPUT);
  digitalWrite(LIGA_DIVISOR, HIGH);
  delay(2);
  uint32_t mv = 0;
  for (int i = 0; i < 8; i++) {
    mv += analogReadMilliVolts(PINO_BATERIA);   // já calibrado pelo eFuse
  }
  digitalWrite(LIGA_DIVISOR, LOW);
  Leitura l = {++envios, (uint16_t)(mv / 8 * 2)};

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  esp_now_init();
  esp_now_peer_info_t par = {};
  memcpy(par.peer_addr, DIFUSAO, 6);
  par.channel = 6;
  esp_now_add_peer(&par);
  esp_now_send(DIFUSAO, (uint8_t *)&l, sizeof(l));
  delay(20);                            // deixa o quadro sair antes de desligar
  WiFi.mode(WIFI_OFF);

  gpio_hold_en((gpio_num_t)LIGA_DIVISOR);  // MOSFET desligado no sono
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
  gpio_deep_sleep_hold_en();
#endif
  esp_sleep_enable_timer_wakeup(PERIODO_US);
  esp_deep_sleep_start();
}

void loop() {}
