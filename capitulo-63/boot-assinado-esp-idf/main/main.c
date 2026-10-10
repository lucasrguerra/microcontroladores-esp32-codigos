#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_secure_boot.h"
#include "esp_app_desc.h"
#include "soc/soc_caps.h"

static const char *TAG = "boot";

void app_main(void)
{
    const esp_app_desc_t *app = esp_app_get_description();
    ESP_LOGI(TAG, "%s versão %s", app->project_name, app->version);
    ESP_LOGI(TAG, "Secure Boot: %s",
             esp_secure_boot_enabled() ? "ligado" : "desligado");
#if SOC_SUPPORT_SECURE_BOOT_REVOKE_KEY
    for (int i = 0; i < SOC_EFUSE_SECURE_BOOT_KEY_DIGESTS; i++) {
        ESP_LOGI(TAG, "resumo de chave %d: %s", i,
                 esp_efuse_get_digest_revoke(i) ? "revogado" : "válido");
    }
#endif
}
