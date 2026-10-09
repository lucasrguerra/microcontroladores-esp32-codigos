#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

const uint8_t TODOS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const int CANAL = 6;                    // o mesmo em todas as placas

typedef struct __attribute__((packed)) {
  uint32_t contador;
  uint16_t leitura;
} pacote_t;

volatile bool chegou = false;
pacote_t ultimo;
int rssi_ultimo;
uint8_t de[6];

void ao_receber(const esp_now_recv_info_t *info, const uint8_t *dados, int n) {
  if (n == sizeof(pacote_t)) {
    memcpy(&ultimo, dados, n);
    memcpy(de, info->src_addr, 6);
    rssi_ultimo = info->rx_ctrl->rssi;
    chegou = true;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE);
  if (esp_now_init() != ESP_OK) {
    Serial.println("falha ao iniciar o ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(ao_receber);
  esp_now_peer_info_t par = {};
  memcpy(par.peer_addr, TODOS, 6);
  par.channel = CANAL;
  par.encrypt = false;                  // difusão não pode ser cifrada
  esp_now_add_peer(&par);
  Serial.printf("meu MAC: %s\n", WiFi.macAddress().c_str());
}

void loop() {
  static uint32_t contador = 0, antes = 0;
  if (millis() - antes >= 2000) {
    antes = millis();
    pacote_t p = {++contador, (uint16_t)analogRead(A0)};
    esp_now_send(TODOS, (const uint8_t *)&p, sizeof p);
  }
  if (chegou) {
    chegou = false;
    Serial.printf("de %02X:%02X:%02X:%02X:%02X:%02X  #%lu  leitura %u  RSSI %d\n",
                  de[0], de[1], de[2], de[3], de[4], de[5],
                  (unsigned long)ultimo.contador, ultimo.leitura, rssi_ultimo);
  }
}
