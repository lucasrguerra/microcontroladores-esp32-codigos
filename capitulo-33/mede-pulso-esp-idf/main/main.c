#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "driver/mcpwm_cap.h"

#define SAIDA    4                  /* LEDC gera aqui...           */
#define ENTRADA  5                  /* ...e a captura mede aqui */

typedef struct { uint32_t periodo, alto; } medida_t;
static QueueHandle_t s_caixa;

static bool IRAM_ATTR na_borda(mcpwm_cap_channel_handle_t c,
                               const mcpwm_capture_event_data_t *ev, void *arg)
{
    static uint32_t subida_anterior, subida;
    BaseType_t acordou = pdFALSE;
    if (ev->cap_edge == MCPWM_CAP_EDGE_POS) {
        subida_anterior = subida;
        subida = ev->cap_value;
    } else {
        medida_t m = { subida - subida_anterior, ev->cap_value - subida };
        xQueueOverwriteFromISR(s_caixa, &m, &acordou);
    }
    return acordou == pdTRUE;
}

static void gerar_pwm(void)
{
    ledc_timer_config_t t = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));
    ledc_channel_config_t c = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = SAIDA,
        .duty = 307,                   /* 307/1024 = 30 % */
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
}

void app_main(void)
{
    s_caixa = xQueueCreate(1, sizeof(medida_t));
    gerar_pwm();

    mcpwm_cap_timer_handle_t timer;
    mcpwm_capture_timer_config_t tcfg = {
        .group_id = 0,
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_timer(&tcfg, &timer));
    uint32_t hz;
    ESP_ERROR_CHECK(mcpwm_capture_timer_get_resolution(timer, &hz));

    mcpwm_cap_channel_handle_t canal;
    mcpwm_capture_channel_config_t ccfg = {
        .gpio_num = ENTRADA,
        .prescale = 1,
        .flags.pos_edge = true,
        .flags.neg_edge = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_channel(timer, &ccfg, &canal));
    mcpwm_capture_event_callbacks_t cbs = { .on_cap = na_borda };
    ESP_ERROR_CHECK(mcpwm_capture_channel_register_event_callbacks(canal, &cbs,
                                                                   NULL));
    ESP_ERROR_CHECK(mcpwm_capture_channel_enable(canal));
    ESP_ERROR_CHECK(mcpwm_capture_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_capture_timer_start(timer));

    printf("timer de captura a %lu Hz\n", (unsigned long)hz);
    for (;;) {
        medida_t m;
        if (xQueueReceive(s_caixa, &m, pdMS_TO_TICKS(1000)) == pdTRUE && m.periodo) {
            printf("frequência %.2f Hz, ciclo %.2f %%\n",
                   (double)hz / m.periodo, 100.0 * m.alto / m.periodo);
        } else {
            printf("sem sinal no GPIO %d\n", ENTRADA);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
