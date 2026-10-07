#include <stdio.h>
#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp_log.h"

static RTC_DATA_ATTR uint32_t acordadas = 0;   // sobrevive ao deep sleep
static const char *TAG = "rtc";

void app_main(void)
{
    acordadas++;
    ESP_LOGI(TAG, "Acordei %lu vez(es). Motivo: %d",
             (unsigned long)acordadas, (int)esp_sleep_get_wakeup_cause());

    ESP_LOGI(TAG, "Dormindo por 5 segundos...");
    esp_deep_sleep(5 * 1000000ULL);              // em microssegundos
}
