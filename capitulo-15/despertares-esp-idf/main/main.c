#include <stdio.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "soc/soc_caps.h"
#include "nvs_flash.h"

static const char *TAG = "despertares";

#if SOC_RTC_FAST_MEM_SUPPORTED
static RTC_DATA_ATTR uint32_t contador_rtc = 0;
#define ONDE "memória RTC"
#else
#define ONDE "NVS na flash"
#endif

static uint32_t ler_e_incrementar(void)
{
#if SOC_RTC_FAST_MEM_SUPPORTED
    return ++contador_rtc;
#else
    uint32_t n = 0;
    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open("sono", NVS_READWRITE, &h));
    nvs_get_u32(h, "n", &n);          // na primeira vez a chave não existe: n fica 0
    ESP_ERROR_CHECK(nvs_set_u32(h, "n", ++n));
    ESP_ERROR_CHECK(nvs_commit(h));
    nvs_close(h);
    return n;
#endif
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "%s: despertar número %lu (%s)", CONFIG_IDF_TARGET,
             (unsigned long)ler_e_incrementar(), ONDE);

    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(10ULL * 1000000));
    esp_deep_sleep_start();
}
