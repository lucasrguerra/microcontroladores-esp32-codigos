#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gptimer.h"
#include "esp_timer.h"

typedef struct {
    const char *nome;
    int64_t ultimo, menor, maior;
    uint32_t n;
} estat_t;

static estat_t s_gpt = { "GPTimer (1 ms)", 0, INT64_MAX, 0, 0 };
static estat_t s_esp = { "esp_timer (1 ms)", 0, INT64_MAX, 0, 0 };
static estat_t s_rtos = { "timer do FreeRTOS (10 ms)", 0, INT64_MAX, 0, 0 };

static void IRAM_ATTR anotar(estat_t *e)
{
    int64_t t = esp_timer_get_time();
    if (e->ultimo) {
        int64_t d = t - e->ultimo;
        if (d < e->menor) e->menor = d;
        if (d > e->maior) e->maior = d;
    }
    e->ultimo = t;
    e->n++;
}

static bool IRAM_ATTR no_gptimer(gptimer_handle_t t,
                                 const gptimer_alarm_event_data_t *ev, void *arg)
{
    anotar(&s_gpt);
    return false;                       /* nenhuma tarefa acordada */
}

static void no_esp_timer(void *arg) { anotar(&s_esp); }
static void no_rtos(TimerHandle_t t) { anotar(&s_rtos); }

static void mostrar(const estat_t *e)
{
    printf("%-26s %6lu chamadas, intervalo de %5lld a %5lld µs\n", e->nome,
           (unsigned long)e->n, (long long)e->menor, (long long)e->maior);
}

void app_main(void)
{
    gptimer_handle_t gpt;
    gptimer_config_t gcfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,       /* 1 tique = 1 µs */
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&gcfg, &gpt));
    gptimer_event_callbacks_t cbs = { .on_alarm = no_gptimer };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gpt, &cbs, NULL));
    gptimer_alarm_config_t alarme = {
        .alarm_count = 1000,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gpt, &alarme));
    ESP_ERROR_CHECK(gptimer_enable(gpt));

    esp_timer_handle_t et;
    esp_timer_create_args_t eargs = { .callback = no_esp_timer, .name = "medida" };
    ESP_ERROR_CHECK(esp_timer_create(&eargs, &et));

    TimerHandle_t rt = xTimerCreate("medida", pdMS_TO_TICKS(10), pdTRUE, NULL,
                                    no_rtos);

    ESP_ERROR_CHECK(gptimer_start(gpt));
    ESP_ERROR_CHECK(esp_timer_start_periodic(et, 1000));
    xTimerStart(rt, 0);

    vTaskDelay(pdMS_TO_TICKS(10000));   /* dez segundos de medida */

    ESP_ERROR_CHECK(gptimer_stop(gpt));
    ESP_ERROR_CHECK(esp_timer_stop(et));
    xTimerStop(rt, 0);
    mostrar(&s_gpt);
    mostrar(&s_esp);
    mostrar(&s_rtos);
}
