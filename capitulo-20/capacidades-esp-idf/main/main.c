#include <stdio.h>
#include "esp_chip_info.h"
#include "soc/soc_caps.h"
#include "sdkconfig.h"

static void item(const char *nome)
{
    printf("  - %s\n", nome);
}

void app_main(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    printf("%s, %d núcleo(s), revisão v%d.%d, %d GPIOs\n", CONFIG_IDF_TARGET, chip.cores,
           chip.revision / 100, chip.revision % 100, SOC_GPIO_PIN_COUNT);
    printf("Recursos que o ESP-IDF suporta neste chip:\n");

#if SOC_WIFI_SUPPORTED
    item("Wi-Fi 2,4 GHz");
#endif
#if SOC_WIFI_SUPPORT_5G
    item("Wi-Fi 5 GHz");
#endif
#if SOC_WIFI_HE_SUPPORT
    item("Wi-Fi 6 (802.11ax)");
#endif
#if SOC_BT_CLASSIC_SUPPORTED
    item("Bluetooth Clássico");
#endif
#if SOC_BLE_SUPPORTED
    item("Bluetooth LE");
#endif
#if SOC_IEEE802154_SUPPORTED
    item("802.15.4 (Thread e Zigbee)");
#endif
#if SOC_EMAC_SUPPORTED
    item("Ethernet MAC");
#endif
#if SOC_USB_OTG_SUPPORTED
    item("USB OTG");
#endif
#if SOC_USB_SERIAL_JTAG_SUPPORTED
    item("USB Serial/JTAG");
#endif
#if SOC_TWAI_SUPPORTED
    item("TWAI (CAN)");
#endif
#if SOC_SPIRAM_SUPPORTED
    item("PSRAM");
#endif
#if SOC_TOUCH_SENSOR_SUPPORTED
    item("Toque capacitivo");
#endif
#if SOC_DAC_SUPPORTED
    item("DAC");
#endif
#if SOC_ULP_FSM_SUPPORTED || SOC_RISCV_COPROC_SUPPORTED || SOC_LP_CORE_SUPPORTED
    item("Coprocessador de baixo consumo");
#endif
#if SOC_MIPI_CSI_SUPPORTED
    item("Câmera MIPI-CSI");
#endif
}
