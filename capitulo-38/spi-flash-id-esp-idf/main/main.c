#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#if CONFIG_IDF_TARGET_ESP32
#define HOST SPI3_HOST
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5
#elif CONFIG_IDF_TARGET_ESP32S3
#define HOST SPI2_HOST
#define SCK 12
#define MISO 13
#define MOSI 11
#define CS 10
#elif CONFIG_IDF_TARGET_ESP32C6
#define HOST SPI2_HOST
#define SCK 6
#define MISO 2
#define MOSI 7
#define CS 16
#else                                       /* C3 */
#define HOST SPI2_HOST
#define SCK 6
#define MISO 2
#define MOSI 7
#define CS 10
#endif

void app_main(void)
{
    spi_bus_config_t bus = {
        .sclk_io_num = SCK,
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &bus, SPI_DMA_DISABLED));

    spi_device_handle_t flash;
    spi_device_interface_config_t dev = {
        .mode = 0,
        .clock_speed_hz = 10 * 1000 * 1000,
        .spics_io_num = CS,
        .queue_size = 1,
        .command_bits = 8,                  /* fase de comando de 1 byte */
        .flags = SPI_DEVICE_HALFDUPLEX,     /* comando, depois leitura */
    };
    ESP_ERROR_CHECK(spi_bus_add_device(HOST, &dev, &flash));

    int khz;
    ESP_ERROR_CHECK(spi_device_get_actual_freq(flash, &khz));
    printf("relógio real: %d kHz\n", khz);

    for (;;) {
        spi_transaction_t t = {
            .cmd = 0x9F,                    /* Read JEDEC ID */
            .rxlength = 24,                 /* 3 bytes, em bits */
            .flags = SPI_TRANS_USE_RXDATA,  /* resposta em t.rx_data */
        };
        ESP_ERROR_CHECK(spi_device_polling_transmit(flash, &t));
        printf("JEDEC ID: %02X %02X %02X\n",
               t.rx_data[0], t.rx_data[1], t.rx_data[2]);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
