#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define LED        5               /* LED com resistor para o GND */
#define RESOLUCAO  LEDC_TIMER_13_BIT        /* 8192 degraus a 5 kHz */
#define MAXIMO     ((1 << 13) - 1)

static const char *TAG = "fade";
static TaskHandle_t s_tarefa;

static bool IRAM_ATTR fim_do_fade(const ledc_cb_param_t *p, void *arg)
{
    BaseType_t acordou = pdFALSE;
    if (p->event == LEDC_FADE_END_EVT) {
        vTaskNotifyGiveFromISR(s_tarefa, &acordou);
    }
    return acordou == pdTRUE;
}

void app_main(void)
{
    s_tarefa = xTaskGetCurrentTaskHandle();

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = RESOLUCAO,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t canal = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = LED,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&canal));

    ESP_ERROR_CHECK(ledc_fade_func_install(0));
    ledc_cbs_t cbs = { .fade_cb = fim_do_fade };
    ESP_ERROR_CHECK(ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs,
                                     NULL));

    uint32_t alvo = MAXIMO;
    for (;;) {
        ESP_ERROR_CHECK(ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,
                        LEDC_CHANNEL_0, alvo, 2000, LEDC_FADE_NO_WAIT));
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uint32_t d = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ESP_LOGI(TAG, "fade terminou em %lu", (unsigned long)d);
        alvo = alvo ? 0 : MAXIMO;
    }
}
