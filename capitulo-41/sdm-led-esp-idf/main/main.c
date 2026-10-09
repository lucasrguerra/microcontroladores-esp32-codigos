#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/sdm.h"

#define PINO 5                              /* LED com resistor para o GND */

void app_main(void)
{
    sdm_channel_handle_t canal;
    sdm_config_t cfg = {
        .gpio_num = PINO,
        .clk_src = SDM_CLK_SRC_DEFAULT,
        .sample_rate_hz = 1000000,          /* 1 MHz de bits */
    };
    ESP_ERROR_CHECK(sdm_new_channel(&cfg, &canal));
    ESP_ERROR_CHECK(sdm_channel_enable(canal));

    int8_t densidade = -128;
    int passo = 1;
    for (;;) {
        ESP_ERROR_CHECK(sdm_channel_set_pulse_density(canal, densidade));
        if (densidade == 127) passo = -1;
        if (densidade == -128) passo = 1;
        densidade += passo;
        vTaskDelay(pdMS_TO_TICKS(8));       /* ~2 s para ir e voltar */
    }
}
