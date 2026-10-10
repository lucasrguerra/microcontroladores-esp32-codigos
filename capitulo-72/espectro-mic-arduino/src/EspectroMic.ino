// Analisador de espectro: mostra a frequência mais forte que o microfone ouve.
// Microfone I2S INMP441 com os mesmos pinos do Cap. 39; FFT do ESP-DSP.
#include <ESP_I2S.h>
#include "esp_dsp.h"

#if CONFIG_IDF_TARGET_ESP32
const int PINO_BCLK = 26, PINO_WS = 25, PINO_DIN = 33;
#else
const int PINO_BCLK = 4, PINO_WS = 5, PINO_DIN = 6;
#endif
const int N = 1024;                        // 64 ms a 16 kHz; resolução de 15,6 Hz
const float TAXA = 16000;

I2SClass i2s;
int32_t bruto[N];
float janela[N];
__attribute__((aligned(16))) float fft[2 * N];

void setup() {
  Serial.begin(115200);
  i2s.setPins(PINO_BCLK, PINO_WS, -1, PINO_DIN);
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("falha ao iniciar o I2S");
    while (true) delay(1000);
  }
  dsps_wind_hann_f32(janela, N);
  dsps_fft2r_init_fc32(NULL, N);
}

void loop() {
  if (i2s.readBytes((char *)bruto, sizeof(bruto)) != sizeof(bruto)) return;
  for (int i = 0; i < N; i++) {
    fft[2 * i] = (bruto[i] >> 8) / 8388608.0f * janela[i];   // 24 bits, de -1 a 1
    fft[2 * i + 1] = 0;
  }
  dsps_fft2r_fc32(fft, N);
  dsps_bit_rev_fc32(fft, N);
  int pico = 1;
  float maior = 0;
  for (int k = 1; k < N / 2; k++) {        // pula o bin 0: a componente contínua
    float p = fft[2 * k] * fft[2 * k] + fft[2 * k + 1] * fft[2 * k + 1];
    if (p > maior) {
      maior = p;
      pico = k;
    }
  }
  float db = 10 * log10f(maior / (N * N / 16.0f) + 1e-12f);
  Serial.printf("%7.1f Hz  %6.1f dBFS\n", pico * TAXA / N, db);
}
