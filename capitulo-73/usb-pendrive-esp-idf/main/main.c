#include <stdio.h>
#include "esp_log.h"
#include "esp_partition.h"
#include "wear_levelling.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_msc.h"

static const char *TAG = "pendrive";

/* O disco tem um dono por vez: a aplicação ou o computador. */
static void ao_evento(tinyusb_msc_storage_handle_t h, tinyusb_msc_event_t *ev,
                      void *arg)
{
    if (ev->id == TINYUSB_MSC_EVENT_MOUNT_COMPLETE) {
        bool usb = ev->mount_point == TINYUSB_MSC_STORAGE_MOUNT_USB;
        ESP_LOGI(TAG, "disco agora é do %s", usb ? "computador" : "ESP32");
    }
}

void app_main(void)
{
    const esp_partition_t *part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, "dados");
    wl_handle_t wl;
    ESP_ERROR_CHECK(wl_mount(part, &wl));   /* nivelamento de desgaste da flash */

    const tinyusb_msc_driver_config_t driver = {.callback = ao_evento};
    ESP_ERROR_CHECK(tinyusb_msc_install_driver(&driver));
    const tinyusb_msc_storage_config_t disco = {
        .medium.wl_handle = wl,
        .mount_point = TINYUSB_MSC_STORAGE_MOUNT_APP,   /* começa com a aplicação */
        .fat_fs = {.base_path = "/dados",
                   .config = {.max_files = 2, .format_if_mount_failed = true}},
    };
    tinyusb_msc_storage_handle_t h;
    ESP_ERROR_CHECK(tinyusb_msc_new_storage_spiflash(&disco, &h));

    FILE *f = fopen("/dados/leituras.csv", "a");    /* uma linha a cada partida */
    if (f) {
        fprintf(f, "partida;%lu\n", (unsigned long)esp_log_timestamp());
        fclose(f);
    }

    const tinyusb_config_t usb = TINYUSB_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(tinyusb_driver_install(&usb));
    ESP_LOGI(TAG, "conecte a porta USB OTG: o disco passa para o computador");
}
