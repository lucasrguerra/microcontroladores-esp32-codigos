#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_attr.h"
#include "esp_ieee802154.h"
#include "esp_log.h"

static SemaphoreHandle_t s_pronto;
static volatile int8_t s_potencia;

// Chamada pelo driver, dentro da interrupção, ao fim de cada medição
void IRAM_ATTR esp_ieee802154_energy_detect_done(int8_t potencia)
{
    s_potencia = potencia;
    BaseType_t acordou = pdFALSE;
    xSemaphoreGiveFromISR(s_pronto, &acordou);
    portYIELD_FROM_ISR(acordou);
}

void app_main(void)
{
    s_pronto = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(esp_ieee802154_enable());

    for (uint8_t canal = 11; canal <= 26; canal++) {
        esp_ieee802154_set_channel(canal);
        ESP_ERROR_CHECK(esp_ieee802154_energy_detect(500));  // 500 × 16 µs = 8 ms
        xSemaphoreTake(s_pronto, portMAX_DELAY);
        printf("canal %2u: %4d dBm%s\n", canal, s_potencia,
               (canal == 15 || canal == 20 || canal == 25 || canal == 26) ? "  (fora do Wi-Fi 1/6/11)" : "");
    }
    esp_ieee802154_disable();
}
