#include <string.h>
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "ap_sta";

static void ao_evento(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    if (id == WIFI_EVENT_AP_STACONNECTED) {
        const wifi_event_ap_staconnected_t *e = dados;
        ESP_LOGI(TAG, "entrou " MACSTR " (aid %d)", MAC2STR(e->mac), e->aid);
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        const wifi_event_ap_stadisconnected_t *e = dados;
        ESP_LOGI(TAG, "saiu " MACSTR ", motivo %u", MAC2STR(e->mac), e->reason);
    } else if (id == WIFI_EVENT_STA_START || id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();             /* religação simples; veja o Capítulo 49 */
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();        /* 192.168.4.1, com servidor DHCP */
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID, ao_evento, NULL, NULL));

    wifi_config_t ap = {
        .ap = {
            .ssid = "ESP32-config",
            .password = "senha-forte-123",
            .channel = 6,
            .authmode = WIFI_AUTH_WPA2_WPA3_PSK,
            .max_connection = 4,
            .pmf_cfg = { .capable = true },
        },
    };
    wifi_config_t sta = {
        .sta = {
            .ssid = "minha-rede",
            .password = "minha-senha",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "AP ESP32-config no ar; estação tentando minha-rede");
}
