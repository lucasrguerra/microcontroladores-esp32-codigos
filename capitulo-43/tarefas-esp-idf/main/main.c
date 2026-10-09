#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

/* Ocupa a CPU por 'ms' milissegundos, como um cálculo de verdade faria. */
static void ocupar(int ms)
{
    int64_t fim = esp_timer_get_time() + ms * 1000;
    while (esp_timer_get_time() < fim) {
    }
}

static void sensor(void *arg)
{
    TickType_t ultimo = xTaskGetTickCount();
    for (;;) {
        ocupar(2);                                   /* lê e filtra */
        vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(20)); /* 50 vezes por segundo */
    }
}

static void calculo(void *arg)
{
    for (;;) {
        ocupar(300);                                 /* trabalho pesado */
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static char s_texto[1024];

void app_main(void)
{
    const BaseType_t nucleo = configNUMBER_OF_CORES - 1;   /* 1 se houver dois */

    xTaskCreatePinnedToCore(sensor, "sensor", 2048, NULL, 10, NULL, nucleo);
    xTaskCreate(calculo, "calculo", 2048, NULL, 3, NULL);

    vTaskDelay(pdMS_TO_TICKS(5000));

    vTaskList(s_texto);
    printf("Tarefa          Estado Prio Livre  Num Núcleo\n%s\n", s_texto);
    vTaskGetRunTimeStats(s_texto);
    printf("Tarefa          Tempo (µs)      %%\n%s", s_texto);
}
