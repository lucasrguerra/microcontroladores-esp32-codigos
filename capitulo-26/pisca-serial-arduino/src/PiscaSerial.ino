// Pisca um LED sem travar o loop e conta as piscadas no monitor serial.
const int PINO_LED = 2;            // confira no esquema da sua placa
const unsigned long PERIODO = 500; // ms entre trocas de estado

unsigned long ultimaTroca = 0;
bool aceso = false;
uint32_t piscadas = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT);
  Serial.printf("Chip %s, revisão %d, %d núcleo(s) a %lu MHz\n",
                ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash: %lu bytes; heap livre: %lu bytes\n",
                (unsigned long)ESP.getFlashChipSize(),
                (unsigned long)ESP.getFreeHeap());
}

void loop() {
  if (millis() - ultimaTroca >= PERIODO) {
    ultimaTroca = millis();
    aceso = !aceso;
    digitalWrite(PINO_LED, aceso ? HIGH : LOW);
    if (aceso) {
      piscadas++;
      Serial.printf("piscada %lu\n", (unsigned long)piscadas);
    }
  }
}
