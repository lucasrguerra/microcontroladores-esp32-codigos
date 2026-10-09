#include <ESP_I2S.h>
#include <math.h>

#if CONFIG_IDF_TARGET_ESP32
const int PINO_BCLK = 26, PINO_WS = 25, PINO_DIN = 33;   // o GPIO6 do ESP32 é da flash
#else
const int PINO_BCLK = 4, PINO_WS = 5, PINO_DIN = 6;      // SCK, WS e SD do microfone
#endif
const int N = 512;                  // 32 ms a 16 kHz

I2SClass i2s;
int32_t amostras[N];

void setup() {
  Serial.begin(115200);
  i2s.setPins(PINO_BCLK, PINO_WS, -1, PINO_DIN);
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("falha ao iniciar o I2S");
    while (true) delay(1000);
  }
}

void loop() {
  size_t lidos = i2s.readBytes((char *)amostras, sizeof(amostras)) / sizeof(int32_t);
  if (lidos == 0) return;
  double soma = 0;
  for (size_t i = 0; i < lidos; i++) {
    double v = (amostras[i] >> 8) / 8388608.0;   // 24 bits úteis, de -1 a 1
    soma += v * v;
  }
  double rms = sqrt(soma / lidos);
  double dbfs = 20.0 * log10(rms + 1e-9);         // 0 dBFS = escala cheia
  int barra = constrain((int)(dbfs + 90), 0, 60);
  Serial.printf("%6.1f dBFS |", dbfs);
  for (int i = 0; i < barra; i++) Serial.print('#');
  Serial.println();
}
