const int PINO_DAC = 25;   // canal 1 do DAC no ESP32 clássico

void setup() {}

void loop() {
  for (int v = 0; v < 256; v++) {
    dacWrite(PINO_DAC, v);   // 0 = 0 V, 255 = cerca de 3,3 V
    delay(4);                // rampa completa em cerca de 1 s
  }
}
