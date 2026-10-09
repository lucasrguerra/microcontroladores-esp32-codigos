#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"

static const char *TAG = "eventos";

ESP_EVENT_DEFINE_BASE(SENSOR_EVENTOS);
enum { SENSOR_LEITURA, SENSOR_ALARME };

typedef struct {
    int canal;
    float valor;
} leitura_t;

static esp_event_loop_handle_t s_laco;

static void ao_ler(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    const leitura_t *l = dados;
    ESP_LOGI(TAG, "leitura: canal %d = %.1f", l->canal, l->valor);
    if (l->valor > 30.0f) {
        esp_event_post_to(s_laco, SENSOR_EVENTOS, SENSOR_ALARME, l, sizeof(*l), 0);
    }
}

static void ao_alarmar(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    const leitura_t *l = dados;
    ESP_LOGW(TAG, "alarme: canal %d passou de 30 (%.1f)", l->canal, l->valor);
}

static void auditor(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    ESP_LOGI(TAG, "auditor: evento %s, id %ld", base, (long)id);
}

void app_main(void)
{
    esp_event_loop_args_t args = {
        .queue_size = 8,
        .task_name = "sensor_ev",
        .task_priority = 5,
        .task_stack_size = 3072,
        .task_core_id = tskNO_AFFINITY,
    };
    ESP_ERROR_CHECK(esp_event_loop_create(&args, &s_laco));

    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(s_laco,
                    SENSOR_EVENTOS, SENSOR_LEITURA, ao_ler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(s_laco,
                    SENSOR_EVENTOS, SENSOR_ALARME, ao_alarmar, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(s_laco,
                    SENSOR_EVENTOS, ESP_EVENT_ANY_ID, auditor, NULL, NULL));

    const float valores[] = { 22.5f, 31.5f };
    for (int i = 0; i < 2; i++) {
        leitura_t l = { .canal = 1, .valor = valores[i] };
        ESP_ERROR_CHECK(esp_event_post_to(s_laco, SENSOR_EVENTOS, SENSOR_LEITURA,
                                          &l, sizeof(l), portMAX_DELAY));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
