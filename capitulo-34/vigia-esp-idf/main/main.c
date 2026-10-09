#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

static const char *TAG = "vigia";

/* Chamada pela interrupção do watchdog de tarefas, junto com a mensagem padrão. */
void esp_task_wdt_isr_user_handler(void)
{
    ESP_DRAM_LOGE(TAG, "vigia disparou: guarde a causa antes do reinício");
}

void app_main(void)
{
    esp_task_wdt_user_handle_t rede, sensor;
    ESP_ERROR_CHECK(esp_task_wdt_add_user("rede", &rede));
    ESP_ERROR_CHECK(esp_task_wdt_add_user("sensor", &sensor));

    for (int s = 1; ; s++) {
        ESP_ERROR_CHECK(esp_task_wdt_reset_user(sensor));
        if (s <= 8) {
            ESP_ERROR_CHECK(esp_task_wdt_reset_user(rede));
        } else if (s == 9) {
            ESP_LOGW(TAG, "a rede travou: ninguém mais alimenta o usuário \"rede\"");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
