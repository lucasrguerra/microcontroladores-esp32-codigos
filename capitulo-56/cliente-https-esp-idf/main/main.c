#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

static const char *TAG = "https";

static void acertar_relogio(void)
{
    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&cfg);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(15000)) != ESP_OK) {
        ESP_LOGW(TAG, "sem resposta do NTP");
        return;
    }
    setenv("TZ", "<-03>3", 1);                 /* horário de Brasília */
    tzset();
    time_t agora = time(NULL);
    char txt[32];
    strftime(txt, sizeof(txt), "%d/%m/%Y %H:%M:%S", localtime(&agora));
    ESP_LOGI(TAG, "relógio acertado: %s", txt);
}

static void buscar(const char *url)
{
    esp_http_client_config_t cfg = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,  /* CAs públicas embutidas */
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t cli = esp_http_client_init(&cfg);
    int64_t t0 = esp_timer_get_time();
    if (esp_http_client_open(cli, 0) != ESP_OK) {
        ESP_LOGE(TAG, "falha ao conectar em %s", url);
        esp_http_client_cleanup(cli);
        return;
    }
    int64_t t1 = esp_timer_get_time();
    esp_http_client_fetch_headers(cli);
    int status = esp_http_client_get_status_code(cli);

    char buf[512];
    int lidos, total = 0;
    while ((lidos = esp_http_client_read(cli, buf, sizeof(buf))) > 0) {
        total += lidos;
    }
    int64_t t2 = esp_timer_get_time();
    ESP_LOGI(TAG, "HTTP %d, %d bytes", status, total);
    ESP_LOGI(TAG, "conexão + TLS: %lld ms; resposta: %lld ms",
             (t1 - t0) / 1000, (t2 - t1) / 1000);
    esp_http_client_cleanup(cli);
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());    /* Wi-Fi ou Ethernet (menuconfig) */

    acertar_relogio();                     /* SNTP antes do HTTPS */
    buscar("https://example.com/");
    ESP_LOGI(TAG, "menor heap livre: %lu bytes",
             (unsigned long)esp_get_minimum_free_heap_size());
}
