#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_idf_version.h"

static const char *TAG = "sono";

/* Sobrevivem ao deep sleep: ficam na memória RTC (ou LP), que continua ligada. */
static RTC_DATA_ATTR int acordadas;
static RTC_DATA_ATTR int64_t dormiu_em_us;

static int64_t agora_us(void)               /* relógio RTC: segue contando no sono */
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

static bool acordou_pelo_timer(void)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    return esp_sleep_get_wakeup_causes() & BIT(ESP_SLEEP_WAKEUP_TIMER);
#else
    return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
#endif
}

void app_main(void)
{
    if (acordou_pelo_timer()) {
        int64_t ms = (agora_us() - dormiu_em_us) / 1000;
        ESP_LOGI(TAG, "voltou do deep sleep (%d), dormiu %lld ms", acordadas, ms);
        ESP_LOGI(TAG, "o programa recomeçou do zero: app_main de novo");
        return;
    }

    int na_ram = 1234;                      /* variável comum, na SRAM */
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(2 * 1000000));
    ESP_LOGI(TAG, "light sleep por 2 s");
    int64_t t0 = esp_timer_get_time();
    esp_light_sleep_start();                /* volta para a linha seguinte */
    ESP_LOGI(TAG, "acordou: %lld ms depois, na_ram = %d, timer = %d",
             (esp_timer_get_time() - t0) / 1000, na_ram, acordou_pelo_timer());

    acordadas++;
    ESP_LOGI(TAG, "deep sleep por 3 s");
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(3 * 1000000));
    dormiu_em_us = agora_us();
    esp_deep_sleep_start();                 /* não volta: o chip reinicia */
}
