// Sensor de temperatura Zigbee (dispositivo final), ESP32-C6 ou H2.
#include <Arduino.h>
#ifndef ZIGBEE_MODE_ED
#error "Selecione Tools > Zigbee mode > Zigbee ED (end device)"
#endif
#include "Zigbee.h"

const uint8_t ENDPOINT = 10;
const uint8_t BOTAO = BOOT_PIN;

ZigbeeTempSensor sensor(ENDPOINT);

void setup() {
  Serial.begin(115200);
  pinMode(BOTAO, INPUT_PULLUP);

  sensor.setManufacturerAndModel("CienciaEmbarcada", "SensorTemp");
  sensor.setMinMaxValue(10, 50);           // faixa do sensor interno, °C
  sensor.setTolerance(1);
  Zigbee.addEndpoint(&sensor);

  if (!Zigbee.begin()) {                   // ZIGBEE_END_DEVICE por padrão
    Serial.println("Zigbee não iniciou, reiniciando");
    ESP.restart();
  }
  Serial.print("Procurando a rede");
  while (!Zigbee.connected()) {
    Serial.print('.');
    delay(500);
  }
  Serial.println(" conectado");

  // Relata a cada 60 s ou quando variar 0,5 °C.
  sensor.setReporting(0, 60, 0.5);
}

void loop() {
  float t = temperatureRead();
  sensor.setTemperature(t);
  Serial.printf("temperatura %.1f C\n", t);

  // Botão BOOT por 3 s: sai da rede e volta ao estado de fábrica.
  if (digitalRead(BOTAO) == LOW) {
    uint32_t t0 = millis();
    while (digitalRead(BOTAO) == LOW) {
      if (millis() - t0 > 3000) {
        Serial.println("Saindo da rede");
        Zigbee.factoryReset();
      }
      delay(50);
    }
  }
  delay(5000);
}
