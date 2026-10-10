#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "esp_app_desc.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "protocol_examples_common.h"

static const char *TAG = "ota";

static bool autoteste(void)
{
    /* Confira aqui o que a versão nova precisa ter: rede, sensores, servidor. */
    return esp_netif_get_handle_from_ifkey("ETH_DEF") != NULL ||
           esp_netif_get_handle_from_ifkey("WIFI_STA_DEF") != NULL;
}

static void mostra_estado(void)
{
    const esp_partition_t *p = esp_ota_get_running_partition();
    esp_ota_img_states_t estado = ESP_OTA_IMG_UNDEFINED;
    esp_ota_get_state_partition(p, &estado);
    ESP_LOGI(TAG, "rodando %s versão %s, partição %s, estado %d",
             esp_app_get_description()->project_name,
             esp_app_get_description()->version, p->label, (int)estado);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    mostra_estado();
    ESP_ERROR_CHECK(example_connect());

    const esp_partition_t *atual = esp_ota_get_running_partition();
    esp_ota_img_states_t estado;
    if (esp_ota_get_state_partition(atual, &estado) == ESP_OK &&
        estado == ESP_OTA_IMG_PENDING_VERIFY) {
        if (autoteste()) {
            esp_ota_mark_app_valid_cancel_rollback();
            ESP_LOGI(TAG, "autoteste ok: versão confirmada");
        } else {
            ESP_LOGE(TAG, "autoteste falhou: voltando à versão anterior");
            esp_ota_mark_app_invalid_rollback_and_reboot();
        }
        return;
    }

    esp_http_client_config_t http = {
        .url = CONFIG_OTA_URL,
        .crt_bundle_attach = esp_crt_bundle_attach,   /* confere o servidor */
        .timeout_ms = 10000,
    };
    esp_https_ota_config_t cfg = { .http_config = &http };
    esp_https_ota_handle_t h;
    ESP_ERROR_CHECK(esp_https_ota_begin(&cfg, &h));
    esp_app_desc_t novo;
    ESP_ERROR_CHECK(esp_https_ota_get_img_desc(h, &novo));
    if (strcmp(novo.version, esp_app_get_description()->version) == 0) {
        ESP_LOGI(TAG, "servidor também tem a %s: nada a fazer", novo.version);
        esp_https_ota_abort(h);
        return;
    }
    ESP_LOGI(TAG, "baixando a versão %s", novo.version);
    esp_err_t err;
    while ((err = esp_https_ota_perform(h)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
    }
    if (err != ESP_OK || !esp_https_ota_is_complete_data_received(h)) {
        ESP_LOGE(TAG, "download falhou: %s", esp_err_to_name(err));
        esp_https_ota_abort(h);
        return;
    }
    int lidos = esp_https_ota_get_image_len_read(h);
    err = esp_https_ota_finish(h);          /* confere a imagem e troca o boot */
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "imagem recusada: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "%d bytes gravados; reiniciando", lidos);
    esp_restart();
}
