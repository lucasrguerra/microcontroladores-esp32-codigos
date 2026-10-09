#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gpio_etm.h"
#include "driver/gptimer.h"
#include "driver/gptimer_etm.h"
#include "esp_etm.h"

#define PINO 5

void app_main(void)
{
    gpio_config_t io = { .pin_bit_mask = 1ULL << PINO, .mode = GPIO_MODE_OUTPUT };
    ESP_ERROR_CHECK(gpio_config(&io));

    gptimer_handle_t timer;
    gptimer_config_t tcfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&tcfg, &timer));
    gptimer_alarm_config_t alarme = {
        .alarm_count = 1000,                /* 1 ms: o pino troca a cada 1 ms */
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarme));

    esp_etm_event_handle_t evento;
    gptimer_etm_event_config_t ecfg = { .event_type = GPTIMER_ETM_EVENT_ALARM_MATCH };
    ESP_ERROR_CHECK(gptimer_new_etm_event(timer, &ecfg, &evento));

    esp_etm_task_handle_t tarefa;
    gpio_etm_task_config_t gcfg = { .action = GPIO_ETM_TASK_ACTION_TOG };
    ESP_ERROR_CHECK(gpio_new_etm_task(&gcfg, &tarefa));
    ESP_ERROR_CHECK(gpio_etm_task_add_gpio(tarefa, PINO));

    esp_etm_channel_handle_t canal;
    esp_etm_channel_config_t ccfg = {};
    ESP_ERROR_CHECK(esp_etm_new_channel(&ccfg, &canal));
    ESP_ERROR_CHECK(esp_etm_channel_connect(canal, evento, tarefa));
    ESP_ERROR_CHECK(esp_etm_channel_enable(canal));

    ESP_ERROR_CHECK(gptimer_enable(timer));
    ESP_ERROR_CHECK(gptimer_start(timer));
    ESP_ERROR_CHECK(esp_etm_dump(stdout));
    printf("onda de 500 Hz no GPIO %d, sem interrupção\n", PINO);
    for (;;) {
        vTaskDelay(portMAX_DELAY);
    }
}
