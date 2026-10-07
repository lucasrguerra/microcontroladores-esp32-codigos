#include <stdio.h>
#include <inttypes.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_mac.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"

void app_main(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);

    printf("Alvo da compilação: %s\n", CONFIG_IDF_TARGET);
    printf("Revisão de silício: v%d.%d\n", chip.revision / 100, chip.revision % 100);
    printf("Núcleos:            %d\n", chip.cores);
    printf("Rádios:            %s%s%s%s\n",
           (chip.features & CHIP_FEATURE_WIFI_BGN)   ? " Wi-Fi"      : "",
           (chip.features & CHIP_FEATURE_BT)         ? " BT-Clássico" : "",
           (chip.features & CHIP_FEATURE_BLE)        ? " BLE"        : "",
           (chip.features & CHIP_FEATURE_IEEE802154) ? " 802.15.4"   : "");

    uint32_t flash = 0;
    esp_flash_get_size(NULL, &flash);
    printf("Flash:              %" PRIu32 " MB (%s)\n", flash / (1024 * 1024),
           (chip.features & CHIP_FEATURE_EMB_FLASH) ? "embutida" : "externa");
    printf("PSRAM:              %u KB\n",
           (unsigned)(heap_caps_get_total_size(MALLOC_CAP_SPIRAM) / 1024));

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BASE);
    printf("MAC base:           %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
