#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "ulp_lp_core.h"
#include "ulp_main.h"

extern const uint8_t bin_inicio[] asm("_binary_ulp_main_bin_start");
extern const uint8_t bin_fim[] asm("_binary_ulp_main_bin_end");

#define PINO GPIO_NUM_0

static void iniciar_lp(void)
{
    ESP_ERROR_CHECK(rtc_gpio_init(PINO));
    ESP_ERROR_CHECK(rtc_gpio_set_direction(PINO, RTC_GPIO_MODE_INPUT_ONLY));
    ESP_ERROR_CHECK(rtc_gpio_pullup_en(PINO));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(PINO));
    ESP_ERROR_CHECK(ulp_lp_core_load_binary(bin_inicio, bin_fim - bin_inicio));
    ulp_lp_core_cfg_t cfg = {
        .wakeup_source = ULP_LP_CORE_WAKEUP_SOURCE_LP_TIMER,
        .lp_timer_sleep_duration_us = 1000000,   /* roda uma vez por segundo */
    };
    ESP_ERROR_CHECK(ulp_lp_core_run(&cfg));
}

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));        /* tempo para o monitor reconectar */
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_ULP) {
        printf("acordado pelo núcleo LP: %s, %lu rodadas\n",
               ulp_motivo == 1 ? "botão" : "limite de rodadas",
               (unsigned long)ulp_rodadas);
    } else {
        printf("primeiro boot: carregando o programa do núcleo LP\n");
        iniciar_lp();
    }
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    printf("núcleo principal dormindo\n");
    esp_deep_sleep_start();
}
