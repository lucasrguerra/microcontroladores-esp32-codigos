#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

#define PINO_A  4
#define PINO_B  5
#define LIMITE  1000

static const char *TAG = "encoder";

static bool IRAM_ATTR marco(pcnt_unit_handle_t u, const pcnt_watch_event_data_t *ev,
                            void *arg)
{
    ESP_EARLY_LOGI(TAG, "passou por %d", ev->watch_point_value);
    return false;
}

void app_main(void)
{
    pcnt_unit_handle_t unidade;
    pcnt_unit_config_t ucfg = {
        .low_limit = -LIMITE,
        .high_limit = LIMITE,
        .flags.accum_count = true,          /* soma além dos limites */
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&ucfg, &unidade));

    pcnt_glitch_filter_config_t filtro = { .max_glitch_ns = 1000 };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unidade, &filtro));

    pcnt_channel_handle_t ca, cb;
    pcnt_chan_config_t cfg_a = { .edge_gpio_num = PINO_A, .level_gpio_num = PINO_B };
    pcnt_chan_config_t cfg_b = { .edge_gpio_num = PINO_B, .level_gpio_num = PINO_A };
    ESP_ERROR_CHECK(pcnt_new_channel(unidade, &cfg_a, &ca));
    ESP_ERROR_CHECK(pcnt_new_channel(unidade, &cfg_b, &cb));

    /* Canal A: conta nas bordas de A; o nível de B decide o sentido. */
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(ca,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(ca,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    /* Canal B: o contrário, para as quatro bordas contarem. */
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(cb,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(cb,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unidade, LIMITE));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unidade, -LIMITE));
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(unidade, 0));
    pcnt_event_callbacks_t cbs = { .on_reach = marco };
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(unidade, &cbs, NULL));

    ESP_ERROR_CHECK(pcnt_unit_enable(unidade));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unidade));
    ESP_ERROR_CHECK(pcnt_unit_start(unidade));

    for (;;) {
        int conta;
        ESP_ERROR_CHECK(pcnt_unit_get_count(unidade, &conta));
        printf("contagem: %d\n", conta);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
