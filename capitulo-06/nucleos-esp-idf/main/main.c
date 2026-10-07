#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_cpu.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "nucleos";

static void tarefa(void *arg)
{
    const char *nome = pcTaskGetName(NULL);
    for (int i = 0; i < 3; i++) {
        ESP_LOGI(TAG, "%-7s rodando no núcleo %d", nome, esp_cpu_get_core_id());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "FreeRTOS com %d núcleo(s)", CONFIG_FREERTOS_NUMBER_OF_CORES);
    xTaskCreatePinnedToCore(tarefa, "fixa_0", 3072, NULL, 5, NULL, 0);
#if CONFIG_FREERTOS_NUMBER_OF_CORES > 1
    xTaskCreatePinnedToCore(tarefa, "fixa_1", 3072, NULL, 5, NULL, 1);
#endif
    xTaskCreatePinnedToCore(tarefa, "livre", 3072, NULL, 5, NULL, tskNO_AFFINITY);
}
