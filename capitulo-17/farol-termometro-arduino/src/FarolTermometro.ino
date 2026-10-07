#include <BLEDevice.h>

BLEAdvertising *anuncio;

void setup() {
  BLEDevice::init("H2-termometro");
  anuncio = BLEDevice::getAdvertising();
}

void loop() {
  int16_t centesimos = (int16_t)(temperatureRead() * 100);
  String dados;
  dados += (char)0xFF;                       // identificador de empresa 0xFFFF:
  dados += (char)0xFF;                       // reservado para testes
  dados += (char)(centesimos & 0xFF);        // temperatura, byte baixo
  dados += (char)((centesimos >> 8) & 0xFF); // temperatura, byte alto

  BLEAdvertisementData pacote;
  pacote.setName("H2-termometro");
  pacote.setManufacturerData(dados);
  anuncio->setAdvertisementData(pacote);
  anuncio->start();
  delay(5000);                               // anuncia por 5 s
  anuncio->stop();
}
