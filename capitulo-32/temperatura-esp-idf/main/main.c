#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/temperature_sensor.h"
#include "esp_log.h"

static const char *TAG = "temp";

#if SOC_TEMPERATURE_SENSOR_INTR_SUPPORT
static volatile int s_alarme;

static bool IRAM_ATTR passou_limite(temperature_sensor_handle_t t,
            const temperature_sensor_threshold_event_data_t *ev, void *arg)
{
    s_alarme = ev->celsius_value;
    return false;
}
#endif

void app_main(void)
{
    temperature_sensor_handle_t sensor;
    temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    ESP_ERROR_CHECK(temperature_sensor_install(&cfg, &sensor));

#if SOC_TEMPERATURE_SENSOR_INTR_SUPPORT
    temperature_sensor_abs_threshold_config_t limite = {
        .high_threshold = 70,
        .low_threshold = -10,
    };
    ESP_ERROR_CHECK(temperature_sensor_set_absolute_threshold(sensor, &limite));
    temperature_sensor_event_callbacks_t cbs = { .on_threshold = passou_limite };
    ESP_ERROR_CHECK(temperature_sensor_register_callbacks(sensor, &cbs, NULL));
#endif
    ESP_ERROR_CHECK(temperature_sensor_enable(sensor));

    for (;;) {
        float c;
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(sensor, &c));
        ESP_LOGI(TAG, "chip a %.1f °C", c);
#if SOC_TEMPERATURE_SENSOR_INTR_SUPPORT
        if (s_alarme) {
            ESP_LOGW(TAG, "alarme de hardware: %d °C", s_alarme);
            s_alarme = 0;
        }
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
