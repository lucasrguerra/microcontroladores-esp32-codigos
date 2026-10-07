#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define SAIDA    GPIO_NUM_4
#define ENTRADA  GPIO_NUM_5
static const char *TAG = "gpio";

void app_main(void)
{
    gpio_config_t saida = {
        .pin_bit_mask = 1ULL << SAIDA,
        .mode         = GPIO_MODE_INPUT_OUTPUT_OD,   // open-drain, com leitura de volta
        .pull_up_en   = GPIO_PULLUP_ENABLE,           // pull-up interno faz o nível alto
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&saida));

    gpio_config_t entrada = {
        .pin_bit_mask = 1ULL << ENTRADA,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&entrada));
    ESP_ERROR_CHECK(gpio_set_drive_capability(SAIDA, GPIO_DRIVE_CAP_0));   // força mínima

    for (int nivel = 0; nivel <= 1; nivel++) {
        gpio_set_level(SAIDA, nivel);
        vTaskDelay(pdMS_TO_TICKS(1));
        ESP_LOGI(TAG, "escrevi %d | saída lê %d | entrada lê %d",
                 nivel, gpio_get_level(SAIDA), gpio_get_level(ENTRADA));
    }
    gpio_dump_io_configuration(stdout, (1ULL << SAIDA) | (1ULL << ENTRADA));
}
