#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "farejador";
static atomic_uint s_gerencia, s_controle, s_dados;
static atomic_int s_rssi_min = 0;

/* Roda na tarefa do driver Wi-Fi: só conta e sai. */
static void ao_quadro(void *buf, wifi_promiscuous_pkt_type_t tipo)
{
    const wifi_promiscuous_pkt_t *p = buf;
    if (tipo == WIFI_PKT_MGMT) {
        atomic_fetch_add(&s_gerencia, 1);
    } else if (tipo == WIFI_PKT_CTRL) {
        atomic_fetch_add(&s_controle, 1);
    } else if (tipo == WIFI_PKT_DATA) {
        atomic_fetch_add(&s_dados, 1);
    }
    if (p->rx_ctrl.rssi < atomic_load(&s_rssi_min)) {
        atomic_store(&s_rssi_min, p->rx_ctrl.rssi);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));    /* nem estação nem AP */
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_promiscuous_filter_t filtro = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_CTRL |
                       WIFI_PROMIS_FILTER_MASK_DATA,
    };
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filtro));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(ao_quadro));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));

    for (;;) {
        for (uint8_t canal = 1; canal <= 13; canal++) {
            ESP_ERROR_CHECK(esp_wifi_set_channel(canal, WIFI_SECOND_CHAN_NONE));
            atomic_store(&s_gerencia, 0);
            atomic_store(&s_controle, 0);
            atomic_store(&s_dados, 0);
            atomic_store(&s_rssi_min, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));             /* um segundo por canal */
            ESP_LOGI(TAG, "canal %2u: %4u gerência, %4u controle, %4u dados, "
                     "RSSI mínimo %d", canal, atomic_load(&s_gerencia),
                     atomic_load(&s_controle), atomic_load(&s_dados),
                     atomic_load(&s_rssi_min));
        }
    }
}
