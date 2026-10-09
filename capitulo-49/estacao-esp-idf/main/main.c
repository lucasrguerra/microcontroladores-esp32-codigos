#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define SSID   "minha-rede"
#define SENHA  "minha-senha"

#define CONECTADO BIT0

static const char *TAG = "estacao";
static EventGroupHandle_t s_estado;
static esp_timer_handle_t s_religar;
static int s_espera_s = 1;              /* espera antes da próxima tentativa */

static const char *motivo(uint8_t r)
{
    switch (r) {
    case WIFI_REASON_NO_AP_FOUND:          return "rede não encontrada";
    case WIFI_REASON_AUTH_FAIL:            return "autenticação recusada";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:    return "senha errada?";
    case WIFI_REASON_BEACON_TIMEOUT:       return "sinal perdido";
    case WIFI_REASON_ASSOC_LEAVE:          return "desconexão pedida";
    default:                               return "outro";
    }
}

static void religar(void *arg)
{
    esp_wifi_connect();
}

static void ao_evento(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t *d = dados;
        xEventGroupClearBits(s_estado, CONECTADO);
        ESP_LOGW(TAG, "desconectado, motivo %u (%s); nova tentativa em %d s",
                 d->reason, motivo(d->reason), s_espera_s);
        esp_timer_start_once(s_religar, (uint64_t)s_espera_s * 1000000);
        if (s_espera_s < 60) {
            s_espera_s *= 2;            /* 1, 2, 4, 8... até 64 s */
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *ip = dados;
        ESP_LOGI(TAG, "IP " IPSTR ", gateway " IPSTR,
                 IP2STR(&ip->ip_info.ip), IP2STR(&ip->ip_info.gw));
        s_espera_s = 1;
        xEventGroupSetBits(s_estado, CONECTADO);
    }
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();           /* o Wi-Fi guarda dados na NVS */
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    s_estado = xEventGroupCreate();
    esp_timer_create_args_t t = { .callback = religar, .name = "religar" };
    ESP_ERROR_CHECK(esp_timer_create(&t, &s_religar));

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID, ao_evento, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    IP_EVENT_STA_GOT_IP, ao_evento, NULL, NULL));

    wifi_config_t wc = {
        .sta = {
            .ssid = SSID,
            .password = SENHA,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,   /* recusa redes mais fracas */
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,           /* aceita WPA3 também */
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_set_country_code("BR", true));
    ESP_ERROR_CHECK(esp_wifi_start());

    for (;;) {
        xEventGroupWaitBits(s_estado, CONECTADO, pdFALSE, pdTRUE, portMAX_DELAY);
        wifi_ap_record_t ap;
        if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
            ESP_LOGI(TAG, "%s, canal %u, RSSI %d dBm", (char *)ap.ssid,
                     ap.primary, ap.rssi);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
