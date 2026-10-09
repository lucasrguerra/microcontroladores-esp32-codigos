#include <inttypes.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "ola";

void app_main(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);

    uint32_t flash = 0;
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &flash));

    ESP_LOGI(TAG, "Olá! Compilado com o ESP-IDF %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Alvo %s, %d núcleo(s), revisão v%d.%d",
             CONFIG_IDF_TARGET, chip.cores,
             chip.revision / 100, chip.revision % 100);
    ESP_LOGI(TAG, "Flash: %" PRIu32 " MB", flash / (1024 * 1024));
}
