#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_cosine.h"
#include "driver/dac_oneshot.h"
#include "esp_log.h"

static const char *TAG = "dac";

void app_main(void)
{
    dac_cosine_handle_t onda;
    dac_cosine_config_t cfg_onda = {
        .chan_id = DAC_CHAN_0,              /* GPIO25 no ESP32, GPIO17 no S2 */
        .freq_hz = 1000,
        .clk_src = DAC_COSINE_CLK_SRC_DEFAULT,
        .atten = DAC_COSINE_ATTEN_DB_6,     /* metade da amplitude máxima */
        .phase = DAC_COSINE_PHASE_0,
        .offset = 0,
    };
    ESP_ERROR_CHECK(dac_cosine_new_channel(&cfg_onda, &onda));
    ESP_ERROR_CHECK(dac_cosine_start(onda));

    dac_oneshot_handle_t rampa;
    dac_oneshot_config_t cfg_rampa = { .chan_id = DAC_CHAN_1 };  /* GPIO26 / GPIO18 */
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&cfg_rampa, &rampa));

    ESP_LOGI(TAG, "senoide de 1 kHz no canal 0; rampa de 2,56 s no canal 1");
    for (;;) {
        for (int v = 0; v <= 255; v++) {
            ESP_ERROR_CHECK(dac_oneshot_output_voltage(rampa, v));
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
