// Sorteio verdadeiro e resumo SHA-256 com os blocos de hardware do ESP32.
#include "esp_random.h"
#include "mbedtls/sha256.h"

void imprimeHex(const uint8_t *b, size_t n) {
  for (size_t i = 0; i < n; i++) Serial.printf("%02x", b[i]);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  uint8_t token[16];
  esp_fill_random(token, sizeof(token));        // TRNG do chip
  Serial.print("token: ");
  imprimeHex(token, sizeof(token));

  const char *texto = "Microcontroladores ESP32";
  uint8_t hash[32];
  uint32_t t0 = micros();
  mbedtls_sha256((const uint8_t *)texto, strlen(texto), hash, 0);  // 0 = SHA-256
  Serial.printf("sha256 em %lu us: ", micros() - t0);
  imprimeHex(hash, sizeof(hash));
}

void loop() {
  Serial.printf("dado: %ld\n", random(1, 7));    // sem randomSeed(), vem do TRNG
  delay(1000);
}
