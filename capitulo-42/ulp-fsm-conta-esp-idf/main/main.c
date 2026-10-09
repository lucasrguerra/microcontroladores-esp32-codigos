#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "ulp.h"
#include "ulp_main.h"

extern const uint8_t bin_inicio[] asm("_binary_ulp_main_bin_start");
extern const uint8_t bin_fim[] asm("_binary_ulp_main_bin_end");

static void iniciar_ulp(void)
{
    size_t palavras = (bin_fim - bin_inicio) / sizeof(uint32_t);
    ESP_ERROR_CHECK(ulp_load_binary(0, bin_inicio, palavras));
    ESP_ERROR_CHECK(ulp_set_wakeup_period(0, 1000000));   /* 1 s */
    ESP_ERROR_CHECK(ulp_run(&ulp_entry - RTC_SLOW_MEM));
}

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_ULP) {
        /* O ULP FSM só escreve os 16 bits de baixo de cada palavra. */
        printf("acordado pelo ULP FSM: %lu rodadas\n",
               (unsigned long)(ulp_contagem & 0xFFFF));
    } else {
        printf("primeiro boot: carregando o programa do ULP\n");
        iniciar_ulp();
    }
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());
    printf("núcleo principal dormindo\n");
    esp_deep_sleep_start();
}
