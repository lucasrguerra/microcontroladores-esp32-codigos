#include <stdio.h>
#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "esp_log.h"

static const char *TAG = "pinos";

void app_main(void)
{
    // Aqui entraria a inicialização normal da aplicação.

    int saida = 0, entrada = 0;
    for (int i = 0; i < SOC_GPIO_PIN_COUNT; i++) {
        if (!GPIO_IS_VALID_GPIO(i)) {
            continue;
        }
        if (GPIO_IS_VALID_OUTPUT_GPIO(i)) {
            saida++;
        } else {
            entrada++;
            ESP_LOGW(TAG, "GPIO%d: só entrada", i);
        }
    }
    ESP_LOGI(TAG, "%d pinos com saída, %d só de entrada", saida, entrada);

    gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
}
