// Ethernet por SPI com o W5500: funciona em qualquer série.
#include <ETH.h>
#include <SPI.h>

// SCK, MISO, MOSI e SS são os pinos SPI padrão da placa escolhida.
#if CONFIG_IDF_TARGET_ESP32C3
const int PINO_IRQ = 3;
const int PINO_RST = 10;
#else
const int PINO_IRQ = 4;
const int PINO_RST = 5;
#endif

volatile bool conectado = false;

void aoEvento(arduino_event_id_t evento, arduino_event_info_t info) {
  switch (evento) {
    case ARDUINO_EVENT_ETH_START:
      ETH.setHostname("esp32-w5500");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.printf("link ativo: %u Mbit/s, %s\n", ETH.linkSpeed(),
                    ETH.fullDuplex() ? "full duplex" : "half duplex");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("IP ");
      Serial.print(ETH.localIP());
      Serial.print(", gateway ");
      Serial.print(ETH.gatewayIP());
      Serial.print(", DNS ");
      Serial.println(ETH.dnsIP());
      conectado = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("cabo desconectado");
      conectado = false;
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Network.onEvent(aoEvento);

  SPI.begin(SCK, MISO, MOSI);
  // W5500: MAC e PHY no mesmo chip; endereço de PHY 1; SPI a 20 MHz.
  if (!ETH.begin(ETH_PHY_W5500, 1, SS, PINO_IRQ, PINO_RST, SPI, 20)) {
    Serial.println("W5500 não respondeu: confira os fios e a alimentação");
  }
}

void loop() {
  static uint32_t ultimo = 0;
  if (conectado && millis() - ultimo > 10000) {
    ultimo = millis();
    NetworkClient cliente;
    bool ok = cliente.connect("example.com", 80);
    Serial.printf("TCP para example.com:80 %s\n", ok ? "ok" : "falhou");
    cliente.stop();
  }
}
