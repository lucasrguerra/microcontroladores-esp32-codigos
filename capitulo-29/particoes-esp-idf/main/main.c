#include <inttypes.h>
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "sdkconfig.h"

static const char *TAG = "particoes";

static void listar(esp_partition_type_t tipo, const char *nome)
{
    esp_partition_iterator_t it = esp_partition_find(tipo, ESP_PARTITION_SUBTYPE_ANY, NULL);
    for (; it != NULL; it = esp_partition_next(it)) {
        const esp_partition_t *p = esp_partition_get(it);
        ESP_LOGI(TAG, "%-4s %-9s subtipo 0x%02x  0x%06" PRIx32 "  %4" PRIu32 " KB",
                 nome, p->label, p->subtype, p->address, p->size / 1024);
    }
    esp_partition_iterator_release(it);
}

void app_main(void)
{
    listar(ESP_PARTITION_TYPE_APP, "app");
    listar(ESP_PARTITION_TYPE_DATA, "data");

    const esp_partition_t *rodando = esp_ota_get_running_partition();
    const esp_app_desc_t *app = esp_app_get_description();
    char sha[CONFIG_APP_RETRIEVE_LEN_ELF_SHA + 1];
    esp_app_get_elf_sha256(sha, sizeof(sha));

    ESP_LOGI(TAG, "Rodando da partição %s (0x%06" PRIx32 ")",
             rodando->label, rodando->address);
    ESP_LOGI(TAG, "Projeto %s, versão %s, ESP-IDF %s",
             app->project_name, app->version, app->idf_ver);
    ESP_LOGI(TAG, "Compilado em %s %s, SHA-256 do ELF %s...",
             app->date, app->time, sha);
}
