#include <WiFi.h>
#include <WiFiProv.h>

const char *POP = "abcd1234";           // prova de posse: o "PIN" que o app pede
const char *NOME = "PROV_estufa";       // nome do AP de provisionamento

void ao_evento(arduino_event_t *e) {
  switch (e->event_id) {
    case ARDUINO_EVENT_PROV_START:
      Serial.println("provisionamento iniciado: use o app ESP SoftAP Provisioning");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      Serial.printf("recebida a rede %s\n",
                    (const char *)e->event_info.prov_cred_recv.ssid);
      break;
    case ARDUINO_EVENT_PROV_CRED_FAIL:
      Serial.println("falhou: senha errada ou rede não encontrada");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("conectado: %s\n", WiFi.localIP().toString().c_str());
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(ao_evento);
  WiFi.begin();                         // usa a rede guardada, se houver
  WiFiProv.beginProvision(NETWORK_PROV_SCHEME_SOFTAP,
                          NETWORK_PROV_SCHEME_HANDLER_NONE,
                          NETWORK_PROV_SECURITY_1, POP, NOME);
  WiFiProv.printQR(NOME, POP, "softap");
}

void loop() {}
