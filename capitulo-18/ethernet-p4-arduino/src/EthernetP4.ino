#include <ETH.h>

bool conectado = false;

void eventos(arduino_event_id_t evento) {
  if (evento == ARDUINO_EVENT_ETH_GOT_IP) {
    Serial.printf("IP: %s  velocidade: %u Mbps\n",
                  ETH.localIP().toString().c_str(), ETH.linkSpeed());
    conectado = true;
  } else if (evento == ARDUINO_EVENT_ETH_DISCONNECTED) {
    Serial.println("Cabo desconectado");
    conectado = false;
  }
}

void setup() {
  Serial.begin(115200);
  Network.onEvent(eventos);
  ETH.begin();   // PHY e pinos definidos pela placa escolhida no menu
}

void loop() {
  delay(1000);
}
