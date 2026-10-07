#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PINO_LED   GPIO_NUM_4
static const char *TAG = "pisca";

void app_main(void)
{
    gpio_reset_pin(PINO_LED);                         // estado padrão
    gpio_set_direction(PINO_LED, GPIO_MODE_OUTPUT);   // pino como saída
    ESP_LOGI(TAG, "Pisca iniciado no GPIO %d", PINO_LED);

    while (1) {
        gpio_set_level(PINO_LED, 1);                  // LED aceso
        vTaskDelay(pdMS_TO_TICKS(500));               // dorme 500 ms
        gpio_set_level(PINO_LED, 0);                  // LED apagado
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
