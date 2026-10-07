#include <stdio.h>
#include "esp_app_desc.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "identidade";

void app_main(void)
{
    const esp_app_desc_t *app = esp_app_get_description();
    ESP_LOGI(TAG, "Projeto:  %s", app->project_name);
    ESP_LOGI(TAG, "Versão:   %s", app->version);
    ESP_LOGI(TAG, "ESP-IDF:  %s", app->idf_ver);
    ESP_LOGI(TAG, "Compilado em %s às %s", app->date, app->time);

    const esp_partition_t *rodando = esp_ota_get_running_partition();
    ESP_LOGI(TAG, "Rodando da partição '%s' em 0x%08lx",
             rodando->label, (unsigned long)rodando->address);
    ESP_LOGI(TAG, "app_main na tarefa '%s', prioridade %u, núcleo %d",
             pcTaskGetName(NULL), (unsigned)uxTaskPriorityGet(NULL), xPortGetCoreID());
}
