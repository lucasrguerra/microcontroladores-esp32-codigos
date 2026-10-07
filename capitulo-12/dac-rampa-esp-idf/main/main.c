#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
#include "esp_log.h"

void app_main(void)
{
    dac_oneshot_handle_t dac;
    dac_oneshot_config_t cfg = {.chan_id = DAC_CHAN_0};
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&cfg, &dac));
    ESP_LOGI("dac", "Rampa no canal 0 (GPIO 25 no ESP32, GPIO 17 no S2)");

    while (1) {
        for (int v = 0; v < 256; v++) {
            ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac, v));
            vTaskDelay(pdMS_TO_TICKS(4));
        }
    }
}
