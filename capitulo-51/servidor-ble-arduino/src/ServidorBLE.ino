#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

// UUIDs de 128 bits gerados para este exemplo (nunca reutilize os de outro produto).
#define SERVICO     "6e1a0001-4b7c-4a8e-9f00-3c2a5d6e7f80"
#define TEMPERATURA "6e1a0002-4b7c-4a8e-9f00-3c2a5d6e7f80"
#define INTERVALO   "6e1a0003-4b7c-4a8e-9f00-3c2a5d6e7f80"

BLECharacteristic *temperatura;
volatile uint32_t intervalo_ms = 1000;
bool conectado = false;

class AoConectar : public BLEServerCallbacks {
  void onConnect(BLEServer *s) { conectado = true; }
  void onDisconnect(BLEServer *s) {
    conectado = false;
    BLEDevice::startAdvertising();      // volta a anunciar para o próximo
  }
};

class AoEscrever : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) {
    uint32_t v = atoi(c->getValue().c_str());
    if (v >= 100 && v <= 60000) {
      intervalo_ms = v;
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ESP32-ambiente");
  BLEServer *servidor = BLEDevice::createServer();
  servidor->setCallbacks(new AoConectar());

  BLEService *servico = servidor->createService(SERVICO);
  temperatura = servico->createCharacteristic(TEMPERATURA,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  temperatura->addDescriptor(new BLE2902());          // CCCD: liga as notificações
  BLECharacteristic *intervalo = servico->createCharacteristic(INTERVALO,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  intervalo->setCallbacks(new AoEscrever());
  intervalo->setValue("1000");
  servico->start();

  BLEAdvertising *anuncio = BLEDevice::getAdvertising();
  anuncio->addServiceUUID(SERVICO);
  BLEDevice::startAdvertising();
  Serial.println("anunciando como ESP32-ambiente");
}

void loop() {
  static uint32_t ultimo = 0;
  if (conectado && millis() - ultimo >= intervalo_ms) {
    ultimo = millis();
    char texto[8];
    snprintf(texto, sizeof texto, "%.1f", temperatureRead());
    temperatura->setValue(texto);
    temperatura->notify();
  }
}
