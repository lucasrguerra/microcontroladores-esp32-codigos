#include "Arduino.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "misto";

extern "C" void app_main(void)
{
    initArduino();                      /* prepara o core: periféricos, NVS etc. */
    Serial.begin(115200);
    ESP_LOGI(TAG, "Arduino %s sobre o ESP-IDF %s",
             ESP_ARDUINO_VERSION_STR, esp_get_idf_version());

    pinMode(2, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(2, i % 2);         /* API do Arduino... */
        Serial.printf("millis() = %lu, esp_timer = %lld µs\n",
                      millis(), esp_timer_get_time());
        vTaskDelay(pdMS_TO_TICKS(1000));        /* ...e do ESP-IDF, lado a lado */
    }
}
