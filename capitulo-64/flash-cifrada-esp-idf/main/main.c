#include <string.h>
#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_encrypt.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "cifra";
static const char SEGREDO[] = "senha-do-wifi-123";

static void primeiros_bytes(void)
{
    const esp_partition_t *app = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    uint8_t bruto[8], claro[8];
    esp_flash_read(NULL, bruto, app->address, sizeof(bruto));   /* como na flash */
    esp_partition_read(app, 0, claro, sizeof(claro));         /* decifrado */
    ESP_LOG_BUFFER_HEX(TAG, bruto, sizeof(bruto));
    ESP_LOG_BUFFER_HEX(TAG, claro, sizeof(claro));
}

static bool segredo_visivel(void)
{
    const esp_partition_t *p = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs");
    static uint8_t buf[4096];
    for (uint32_t off = 0; off < p->size; off += sizeof(buf)) {
        esp_flash_read(NULL, buf, p->address + off, sizeof(buf));
        if (memmem(buf, sizeof(buf), SEGREDO, strlen(SEGREDO))) {
            return true;
        }
    }
    return false;
}

void app_main(void)
{
    esp_flash_enc_mode_t modo = esp_get_flash_encryption_mode();
    ESP_LOGI(TAG, "flash cifrada: %s (modo %d)",
             esp_flash_encryption_enabled() ? "sim" : "não", (int)modo);
    primeiros_bytes();

    ESP_ERROR_CHECK(nvs_flash_init());         /* cria as chaves da NVS se faltarem */
    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open("rede", NVS_READWRITE, &h));
    char lida[32];
    size_t n = sizeof(lida);
    if (nvs_get_str(h, "senha", lida, &n) != ESP_OK) {
        ESP_ERROR_CHECK(nvs_set_str(h, "senha", SEGREDO));
        ESP_ERROR_CHECK(nvs_commit(h));
        ESP_LOGI(TAG, "senha gravada na NVS");
    } else {
        ESP_LOGI(TAG, "senha lida pela API: %s", lida);
    }
    nvs_close(h);
    ESP_LOGI(TAG, "senha legível na flash bruta? %s",
             segredo_visivel() ? "SIM" : "não");
}
