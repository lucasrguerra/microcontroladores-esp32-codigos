#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_pm.h"
#include "nvs_flash.h"

static const char *TAG = "economia";
#define SSID   "minha-rede"
#define SENHA  "minha-senha"

static esp_pm_lock_handle_t trava_cpu;

static void ao_evento(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "conectado; agora o chip dorme entre os beacons");
    }
}

static void iniciar_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t ini = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&ini));
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, ao_evento, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ao_evento, NULL);

    wifi_config_t cfg = {
        .sta = {
            .listen_interval = 3,      /* acorda a cada 3 beacons (com MAX_MODEM) */
        },
    };
    strcpy((char *)cfg.sta.ssid, SSID);
    strcpy((char *)cfg.sta.password, SENHA);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));
}

static void tarefa_calculo(void *arg)
{
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));   /* bloqueada: o chip pode dormir */
        esp_pm_lock_acquire(trava_cpu);      /* rajada de trabalho a toda */
        volatile uint32_t soma = 0;
        for (uint32_t i = 0; i < 2000000; i++) {
            soma += i;
        }
        esp_pm_lock_release(trava_cpu);
        esp_pm_dump_locks(stdout);           /* tempo em cada frequência e travas */
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_pm_config_t pm = {
        .max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
        .min_freq_mhz = 40,                  /* a do cristal */
        .light_sleep_enable = true,          /* light sleep automático */
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm));
    ESP_ERROR_CHECK(esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "calculo",
                                       &trava_cpu));

    iniciar_wifi();
    xTaskCreate(tarefa_calculo, "calculo", 4096, NULL, 5, NULL);
}
