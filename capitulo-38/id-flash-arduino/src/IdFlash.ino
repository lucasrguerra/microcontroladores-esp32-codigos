#include <SPI.h>

// Pinos do IO MUX (os mais rápidos) de cada série
#if CONFIG_IDF_TARGET_ESP32
const int PINO_SCK = 18, PINO_MISO = 19, PINO_MOSI = 23, PINO_CS = 5;
#elif CONFIG_IDF_TARGET_ESP32S3
const int PINO_SCK = 12, PINO_MISO = 13, PINO_MOSI = 11, PINO_CS = 10;
#elif CONFIG_IDF_TARGET_ESP32C6
const int PINO_SCK = 6, PINO_MISO = 2, PINO_MOSI = 7, PINO_CS = 16;
#else
const int PINO_SCK = 6, PINO_MISO = 2, PINO_MOSI = 7, PINO_CS = 10;   // C3
#endif

void setup() {
  Serial.begin(115200);
  pinMode(PINO_CS, OUTPUT);
  digitalWrite(PINO_CS, HIGH);              // CS em repouso: alto
  SPI.begin(PINO_SCK, PINO_MISO, PINO_MOSI, PINO_CS);
}

void loop() {
  uint8_t id[3];
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PINO_CS, LOW);
  SPI.transfer(0x9F);                       // Read JEDEC ID
  for (int i = 0; i < 3; i++) {
    id[i] = SPI.transfer(0x00);             // envia 0 para receber 1 byte
  }
  digitalWrite(PINO_CS, HIGH);
  SPI.endTransaction();

  Serial.printf("JEDEC ID: fabricante 0x%02X, tipo 0x%02X, capacidade 0x%02X\n",
                id[0], id[1], id[2]);
  if (id[0] == 0x00 || id[0] == 0xFF) {
    Serial.println("  sem resposta: confira fiação, CS e alimentação");
  }
  delay(2000);
}
