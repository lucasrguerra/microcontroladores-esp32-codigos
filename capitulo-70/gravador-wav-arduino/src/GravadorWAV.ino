// Gravador pela rede: abra http://<ip>/gravar e o navegador recebe 3 s de áudio.
// Microfone I2S INMP441 com os mesmos pinos do Cap. 39.
#include <WiFi.h>
#include <WebServer.h>
#include <ESP_I2S.h>

const char *REDE = "minha-rede";
const char *SENHA = "minha-senha";
#if CONFIG_IDF_TARGET_ESP32
const int PINO_BCLK = 26, PINO_WS = 25, PINO_DIN = 33;
#else
const int PINO_BCLK = 4, PINO_WS = 5, PINO_DIN = 6;
#endif

I2SClass i2s;
WebServer servidor(80);

void gravar() {
  size_t tamanho = 0;
  uint8_t *wav = i2s.recordWAV(3, &tamanho);             // 3 s: cerca de 96 KB
  if (!wav) {
    servidor.send(500, "text/plain", "sem memória para gravar");
    return;
  }
  servidor.send_P(200, "audio/wav", (const char *)wav, tamanho);
  free(wav);                                       // o recordWAV aloca; você libera
}

void setup() {
  Serial.begin(115200);
  i2s.setPins(PINO_BCLK, PINO_WS, -1, PINO_DIN);
  // O INMP441 manda 24 bits num quadro de 32; o Arduino Core converte para 16.
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO) ||
      !i2s.configureRX(16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO,
                       I2S_RX_TRANSFORM_32_TO_16)) {
    Serial.println("falha ao iniciar o I2S");
    while (true) delay(1000);
  }
  WiFi.begin(REDE, SENHA);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  servidor.on("/gravar", gravar);
  servidor.begin();
  Serial.printf("abra http://%s/gravar\n", WiFi.localIP().toString().c_str());
}

void loop() {
  servidor.handleClient();
}
