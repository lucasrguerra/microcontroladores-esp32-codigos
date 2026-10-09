#include <BluetoothSerial.h>

BluetoothSerial bt;

void setup() {
  Serial.begin(115200);
  bt.begin("ESP32-serial");             // nome que aparece no pareamento
  Serial.println("pareie pelo celular e abra um terminal Bluetooth (SPP)");
}

void loop() {
  while (bt.available()) {              // o que chega pelo Bluetooth vai para a USB
    Serial.write(bt.read());
  }
  while (Serial.available()) {          // e o contrário
    bt.write(Serial.read());
  }
}
