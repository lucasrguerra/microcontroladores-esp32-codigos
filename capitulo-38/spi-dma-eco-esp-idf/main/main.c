#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"

#if CONFIG_IDF_TARGET_ESP32
#define HOST SPI3_HOST
#define SCK 18
#define MISO 19
#define MOSI 23
#elif CONFIG_IDF_TARGET_ESP32S3
#define HOST SPI2_HOST
#define SCK 12
#define MISO 13
#define MOSI 11
#else
#define HOST SPI2_HOST
#define SCK 6
#define MISO 2
#define MOSI 7
#endif
#define TAMANHO 4096
#define VEZES   100

void app_main(void)
{
    spi_bus_config_t bus = {
        .sclk_io_num = SCK,
        .mosi_io_num = MOSI,                /* ligue o MOSI ao MISO */
        .miso_io_num = MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TAMANHO,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &bus, SPI_DMA_CH_AUTO));

    spi_device_handle_t eco;
    spi_device_interface_config_t dev = {
        .mode = 0,
        .clock_speed_hz = 40 * 1000 * 1000,
        .spics_io_num = -1,                 /* sem CS: só o fio de eco */
        .queue_size = 2,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(HOST, &dev, &eco));

    uint8_t *tx = heap_caps_malloc(TAMANHO, MALLOC_CAP_DMA);
    uint8_t *rx = heap_caps_malloc(TAMANHO, MALLOC_CAP_DMA);
    for (int i = 0; i < TAMANHO; i++) {
        tx[i] = (uint8_t)(i * 7 + 1);
    }

    for (;;) {
        int erros = 0;
        int64_t t0 = esp_timer_get_time();
        for (int v = 0; v < VEZES; v++) {
            memset(rx, 0, TAMANHO);
            spi_transaction_t t = {
                .length = TAMANHO * 8,      /* em bits */
                .tx_buffer = tx,
                .rx_buffer = rx,
            };
            ESP_ERROR_CHECK(spi_device_transmit(eco, &t));
            erros += memcmp(tx, rx, TAMANHO) != 0;
        }
        int64_t us = esp_timer_get_time() - t0;
        double mbs = (double)TAMANHO * VEZES / us;   /* bytes por µs = MB/s */
        printf("%d transações de %d bytes em %lld µs: %.2f MB/s, %d com erro\n",
               VEZES, TAMANHO, (long long)us, mbs, erros);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
