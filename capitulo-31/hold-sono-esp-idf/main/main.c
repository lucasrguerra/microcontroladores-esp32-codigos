#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/soc_caps.h"

#define LED  GPIO_NUM_5            /* LED com resistor para o GND */

static const char *TAG = "hold";
static RTC_DATA_ATTR int s_nivel;  /* sobrevive ao deep sleep */

void app_main(void)
{
    ESP_LOGI(TAG, "acordei (causa %d), LED estava em %d",
             (int)esp_sleep_get_wakeup_cause(), s_nivel);

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << LED,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_ERROR_CHECK(gpio_set_level(LED, s_nivel));
    ESP_ERROR_CHECK(gpio_hold_dis(LED));

    vTaskDelay(pdMS_TO_TICKS(2000));
    s_nivel = !s_nivel;
    ESP_ERROR_CHECK(gpio_set_level(LED, s_nivel));
    ESP_ERROR_CHECK(gpio_hold_en(LED));
#if !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
    gpio_deep_sleep_hold_en();
#endif

    ESP_LOGI(TAG, "LED em %d; dormindo 5 s", s_nivel);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(5 * 1000 * 1000));
    esp_deep_sleep_start();
}
