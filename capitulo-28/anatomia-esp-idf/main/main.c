#include "esp_log.h"
#include "sdkconfig.h"
#include "pisca.h"

static const char *TAG = "anatomia";

void app_main(void)
{
    ESP_LOGI(TAG, "LED no GPIO %d, trocando a cada %d ms",
             CONFIG_ANATOMIA_GPIO_LED, CONFIG_ANATOMIA_PERIODO_MS);
    ESP_ERROR_CHECK(pisca_iniciar(CONFIG_ANATOMIA_GPIO_LED,
                                  CONFIG_ANATOMIA_PERIODO_MS));
}
