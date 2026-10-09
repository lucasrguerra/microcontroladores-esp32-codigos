#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MAX_REDES 20

static const char *TAG = "rssi";

static void iniciar_wifi(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    iniciar_wifi();
    static wifi_ap_record_t redes[MAX_REDES];
    while (1) {
        ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
        uint16_t n = MAX_REDES;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&n, redes));
        ESP_LOGI(TAG, "%u redes", n);
        for (int i = 0; i < n; i++) {
            ESP_LOGI(TAG, "%4d dBm  canal %2u  %s",
                     redes[i].rssi, redes[i].primary, (const char *)redes[i].ssid);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
