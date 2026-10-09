#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"

static const char *TAG = "cartao";

#if CONFIG_IDF_TARGET_ESP32
#define PINO_MOSI 23
#define PINO_MISO 19
#define PINO_SCLK 18
#define PINO_CS   5
#else
#define PINO_MOSI 6
#define PINO_MISO 5
#define PINO_SCLK 4
#define PINO_CS   7
#endif

void app_main(void)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus = {
        .mosi_io_num = PINO_MOSI,
        .miso_io_num = PINO_MISO,
        .sclk_io_num = PINO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host.slot, &bus, SDSPI_DEFAULT_DMA));

    sdspi_device_config_t disp = SDSPI_DEVICE_CONFIG_DEFAULT();
    disp.gpio_cs = PINO_CS;
    disp.host_id = host.slot;

    esp_vfs_fat_sdmmc_mount_config_t cfg = {
        .format_if_mount_failed = false,        /* nunca apague o cartão do usuário */
        .max_files = 4,
        .allocation_unit_size = 16 * 1024,
    };
    sdmmc_card_t *cartao;
    esp_err_t err = esp_vfs_fat_sdspi_mount("/sd", &host, &disp, &cfg, &cartao);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "falha ao montar o cartão: %s", esp_err_to_name(err));
        return;
    }
    sdmmc_card_print_info(stdout, cartao);

    FILE *f = fopen("/sd/ola.txt", "w");
    if (f) {
        fprintf(f, "gravado pelo ESP32\n");
        fclose(f);
        ESP_LOGI(TAG, "arquivo gravado");
    }
    ESP_ERROR_CHECK(esp_vfs_fat_sdcard_unmount("/sd", cartao));
    ESP_ERROR_CHECK(spi_bus_free(host.slot));
}
