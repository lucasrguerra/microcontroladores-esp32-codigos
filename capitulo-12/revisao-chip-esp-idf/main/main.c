#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_psram.h"
#include "sdkconfig.h"

void app_main(void)
{
    esp_chip_info_t info;
    esp_chip_info(&info);

    printf("%s, revisão v%d.%d, %d núcleo(s)\n", CONFIG_IDF_TARGET,
           info.revision / 100, info.revision % 100, info.cores);
    printf("Wi-Fi: %s, Bluetooth Classic: %s, BLE: %s\n",
           (info.features & CHIP_FEATURE_WIFI_BGN) ? "sim" : "não",
           (info.features & CHIP_FEATURE_BT) ? "sim" : "não",
           (info.features & CHIP_FEATURE_BLE) ? "sim" : "não");

    if (info.revision < 300) {
        printf("Revisão anterior à v3.0: veja a errata CPU-3.2 e o contorno de PSRAM.\n");
    }

    uint32_t flash = 0;
    esp_flash_get_size(NULL, &flash);
    printf("Flash: %lu MB\n", (unsigned long)(flash / (1024 * 1024)));
#if CONFIG_SPIRAM
    printf("PSRAM: %u bytes\n", (unsigned)esp_psram_get_size());
#endif
}
