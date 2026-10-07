#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#define AMOSTRAS 1000
static const char *TAG = "jitter";
static int64_t instantes[AMOSTRAS];
static volatile int n = 0;
static TaskHandle_t tarefa_principal;

static void ao_disparar(void *arg)
{
    if (n < AMOSTRAS) {
        instantes[n++] = esp_timer_get_time();
        if (n == AMOSTRAS) {
            xTaskNotifyGive(tarefa_principal);   // avisa que terminou
        }
    }
}

void app_main(void)
{
    tarefa_principal = xTaskGetCurrentTaskHandle();
    const esp_timer_create_args_t args = { .callback = ao_disparar, .name = "periodo_1ms" };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000));   // 1000 µs
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    esp_timer_stop(timer);

    int64_t menor = INT64_MAX, maior = 0, soma = 0;
    for (int i = 1; i < AMOSTRAS; i++) {
        int64_t d = instantes[i] - instantes[i - 1];
        if (d < menor) menor = d;
        if (d > maior) maior = d;
        soma += d;
    }
    ESP_LOGI(TAG, "intervalo médio: %" PRId64 " us", soma / (AMOSTRAS - 1));
    ESP_LOGI(TAG, "menor: %" PRId64 " us, maior: %" PRId64 " us, jitter: %" PRId64 " us",
             menor, maior, maior - menor);
}
