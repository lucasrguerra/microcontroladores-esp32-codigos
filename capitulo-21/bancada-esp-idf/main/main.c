#include <inttypes.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_flash.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"

static const char *TAG = "bancada";

static const char *motivo(esp_reset_reason_t r)
{
    switch (r) {
    case ESP_RST_POWERON:  return "liga e desliga (power-on)";
    case ESP_RST_EXT:      return "pino EN";
    case ESP_RST_SW:       return "esp_restart()";
    case ESP_RST_PANIC:    return "pânico";
    case ESP_RST_INT_WDT:  return "watchdog de interrupção";
    case ESP_RST_TASK_WDT: return "watchdog de tarefa";
    case ESP_RST_BROWNOUT: return "brownout (tensão baixa)";
    case ESP_RST_DEEPSLEEP: return "saída do deep sleep";
    default:               return "outro";
    }
}

void app_main(void)
{
    esp_reset_reason_t r = esp_reset_reason();
    ESP_LOGI(TAG, "motivo do reset: %s (%d)", motivo(r), r);

    uint32_t strap = REG_READ(GPIO_STRAP_REG);
    ESP_LOGI(TAG, "strapping lido no boot: 0x%08" PRIx32, strap);

    uint32_t fisico = 0;
    esp_flash_get_physical_size(NULL, &fisico);
    ESP_LOGI(TAG, "flash física: %" PRIu32 " MB", fisico / (1024 * 1024));

    if (r == ESP_RST_BROWNOUT) {
        ESP_LOGW(TAG, "a fonte afundou: confira os 10 µF e a corrente do regulador");
    }
}
