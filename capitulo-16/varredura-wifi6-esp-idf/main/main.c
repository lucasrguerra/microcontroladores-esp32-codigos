#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "soc/soc_caps.h"

#define MAX_REDES 20

void app_main(void)
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
#if SOC_WIFI_SUPPORT_5G
    ESP_ERROR_CHECK(esp_wifi_set_band_mode(WIFI_BAND_MODE_AUTO));
#endif
    ESP_ERROR_CHECK(esp_wifi_start());

    static wifi_ap_record_t redes[MAX_REDES];
    uint16_t n = MAX_REDES;
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&n, redes));

    printf("%s: %u rede(s)\n", CONFIG_IDF_TARGET, n);
    for (int i = 0; i < n; i++) {
        const wifi_ap_record_t *r = &redes[i];
        printf("%-24s canal %3u  %-7s  %4d dBm  %s\n",
               (const char *)r->ssid, r->primary,
               r->primary > 14 ? "5 GHz" : "2,4 GHz", r->rssi,
               r->phy_11ax ? "802.11ax" :
               r->phy_11n ? "802.11n" : "802.11b/g");
    }
}
