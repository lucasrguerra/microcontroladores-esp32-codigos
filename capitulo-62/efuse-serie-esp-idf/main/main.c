#include <inttypes.h>
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_flash_encrypt.h"
#include "esp_secure_boot.h"
#include "hal/efuse_hal.h"

static const char *TAG = "efuse";

/* Campo próprio: 32 bits no início do bloco de usuário (BLOCK3). */
static const esp_efuse_desc_t SERIE_BITS = { EFUSE_BLK3, 0, 32 };
static const esp_efuse_desc_t *SERIE[] = { &SERIE_BITS, NULL };

static void identidade(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    unsigned rev = efuse_hal_chip_revision();
    ESP_LOGI(TAG, "MAC de fábrica " MACSTR ", revisão v%u.%u",
             MAC2STR(mac), rev / 100, rev % 100);
    ESP_LOGI(TAG, "criptografia da flash: %s; Secure Boot: %s",
             esp_flash_encryption_enabled() ? "ligada" : "desligada",
             esp_secure_boot_enabled() ? "ligado" : "desligado");
}

static void blocos_de_chave(void)
{
    for (int b = EFUSE_BLK_KEY0; b < EFUSE_BLK_KEY_MAX; b++) {
        ESP_LOGI(TAG, "BLOCK_KEY%d: %-6s propósito %2d, leitura %s",
                 b - EFUSE_BLK_KEY0,
                 esp_efuse_key_block_unused(b) ? "livre," : "usado,",
                 (int)esp_efuse_get_key_purpose(b),
                 esp_efuse_get_key_dis_read(b) ? "bloqueada" : "liberada");
    }
}

void app_main(void)
{
    identidade();
    blocos_de_chave();

    uint32_t serie = 0;
    ESP_ERROR_CHECK(esp_efuse_read_field_blob(SERIE, &serie, 32));
    if (serie == 0) {
        serie = 0x26100001;                       /* lote 2610, unidade 1 */
        ESP_ERROR_CHECK(esp_efuse_write_field_blob(SERIE, &serie, 32));
        ESP_LOGW(TAG, "série gravada: %08" PRIx32 "; reiniciando", serie);
        esp_restart();
    }
    ESP_LOGI(TAG, "série lida depois do reset: %08" PRIx32, serie);

    uint32_t outra = 0x26100002;                  /* tentar regravar */
    esp_err_t err = esp_efuse_write_field_blob(SERIE, &outra, 32);
    ESP_LOGW(TAG, "regravar: %s", esp_err_to_name(err));
    esp_efuse_read_field_blob(SERIE, &serie, 32);
    ESP_LOGI(TAG, "valor agora: %08" PRIx32, serie);
}
