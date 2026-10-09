#include <WiFi.h>

const char *SSID = "minha-rede";
const char *SENHA = "minha-senha";

void ao_evento(WiFiEvent_t evento, WiFiEventInfo_t info) {
  switch (evento) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("IP %s, RSSI %d dBm, canal %d\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI(), WiFi.channel());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.printf("desconectado, motivo %u (%s)\n",
                    info.wifi_sta_disconnected.reason,
                    WiFi.disconnectReasonName(
                        (wifi_err_reason_t)info.wifi_sta_disconnected.reason));
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(ao_evento);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);          // o core religa sozinho
  WiFi.begin(SSID, SENHA);
}

void loop() {
  static uint32_t ultimo = 0;
  if (millis() - ultimo >= 10000) {
    ultimo = millis();
    if (WiFi.isConnected()) {
      Serial.printf("conectado há %lu s, RSSI %d dBm\n",
                    millis() / 1000, WiFi.RSSI());
    } else {
      Serial.println("sem conexão");
    }
  }
}
