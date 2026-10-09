#include <BLEDevice.h>

// O serviço e a característica do ServidorBLE do Capítulo 51.
static BLEUUID SERVICO("6e1a0001-4b7c-4a8e-9f00-3c2a5d6e7f80");
static BLEUUID TEMPERATURA("6e1a0002-4b7c-4a8e-9f00-3c2a5d6e7f80");

BLEAdvertisedDevice *alvo = nullptr;
bool desconectou = true;

class AoAchar : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) {
    if (d.haveServiceUUID() && d.isAdvertisingService(SERVICO)) {
      BLEDevice::getScan()->stop();
      alvo = new BLEAdvertisedDevice(d);
    }
  }
};

class AoCair : public BLEClientCallbacks {
  void onConnect(BLEClient *c) {}
  void onDisconnect(BLEClient *c) { desconectou = true; }
};

void ao_notificar(BLERemoteCharacteristic *c, uint8_t *dados, size_t n, bool notif) {
  Serial.printf("temperatura: %.*s °C\n", (int)n, (const char *)dados);
}

bool conectar() {
  BLEClient *cliente = BLEDevice::createClient();
  cliente->setClientCallbacks(new AoCair());
  if (!cliente->connect(alvo)) {
    return false;
  }
  cliente->setMTU(247);                 // pede pacotes maiores
  BLERemoteService *s = cliente->getService(SERVICO);
  BLERemoteCharacteristic *t = s ? s->getCharacteristic(TEMPERATURA) : nullptr;
  if (t == nullptr || !t->canNotify()) {
    cliente->disconnect();
    return false;
  }
  t->registerForNotify(ao_notificar);   // escreve no CCCD do servidor
  Serial.printf("conectado a %s\n", alvo->getAddress().toString().c_str());
  return true;
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  BLEScan *varredura = BLEDevice::getScan();
  varredura->setAdvertisedDeviceCallbacks(new AoAchar());
  varredura->setActiveScan(true);
}

void loop() {
  if (desconectou) {
    if (alvo == nullptr) {
      BLEDevice::getScan()->start(5, false);       // procura por 5 s
    } else {
      desconectou = !conectar();
      if (desconectou) {
        delete alvo;
        alvo = nullptr;
      }
    }
  }
  delay(1000);
}
