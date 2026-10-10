// Lê o que os eFuses dizem sobre o chip. Nenhuma gravação.
#include "esp_efuse.h"
#include "esp_flash_encrypt.h"
#include "esp_secure_boot.h"
#include "soc/soc_caps.h"

void setup() {
  Serial.begin(115200);
  delay(500);

  uint64_t mac = ESP.getEfuseMac();               // MAC de fábrica, do BLOCK1
  Serial.printf("%s rev. %d, MAC %012llx\n", ESP.getChipModel(),
                ESP.getChipRevision(), mac);
  Serial.printf("criptografia da flash: %s\n",
                esp_flash_encryption_enabled() ? "ligada" : "desligada");
  Serial.printf("Secure Boot: %s\n",
                esp_secure_boot_enabled() ? "ligado" : "desligado");

  uint32_t usuario = 0;                           // primeiros 32 bits do BLOCK3
  esp_efuse_read_block(EFUSE_BLK3, &usuario, 0, 32);
  Serial.printf("BLOCK3[0..31] = %08lx\n", (unsigned long)usuario);

#if SOC_EFUSE_KEY_PURPOSE_FIELD
  for (int b = EFUSE_BLK_KEY0; b < EFUSE_BLK_KEY_MAX; b++) {
    esp_efuse_block_t blk = (esp_efuse_block_t)b;
    Serial.printf("KEY%d: propósito %d%s\n", b - EFUSE_BLK_KEY0,
                  (int)esp_efuse_get_key_purpose(blk),
                  esp_efuse_get_key_dis_read(blk) ? ", protegida" : "");
  }
#endif
}

void loop() {}
