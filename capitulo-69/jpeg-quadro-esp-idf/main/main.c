#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_jpeg_enc.h"

#define LARG 320
#define ALT  240
static const char *TAG = "jpeg";

static uint16_t rgb565(int r, int g, int b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/* Quadro de teste: degradê em cima, barras de cor no meio, xadrez fino embaixo. */
static void quadro_de_teste(uint16_t *px)
{
    static const uint8_t barras[8][3] = {
        {255, 255, 255}, {255, 255, 0}, {0, 255, 255}, {0, 255, 0},
        {255, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 0}};
    for (int y = 0; y < ALT; y++) {
        for (int x = 0; x < LARG; x++) {
            uint16_t c;
            if (y < 80) {
                c = rgb565(x * 255 / LARG, y * 3, 255 - x * 255 / LARG);
            } else if (y < 160) {
                const uint8_t *b = barras[x / (LARG / 8)];
                c = rgb565(b[0], b[1], b[2]);
            } else {
                c = ((x / 4 + y / 4) & 1) ? 0xFFFF : 0x0000;
            }
            px[y * LARG + x] = c;
        }
    }
}

void app_main(void)
{
    uint16_t *px = malloc(LARG * ALT * 2);
    uint8_t *jpg = malloc(48 * 1024);
    quadro_de_teste(px);
    ESP_LOGI(TAG, "quadro bruto: %d bytes", LARG * ALT * 2);

    jpeg_enc_config_t cfg = DEFAULT_JPEG_ENC_CONFIG();
    cfg.width = LARG;
    cfg.height = ALT;
    cfg.src_type = JPEG_PIXEL_FORMAT_RGB565_LE;
    cfg.subsampling = JPEG_SUBSAMPLE_420;          /* cor em meia resolução */
    const int qualidades[] = {90, 60, 30, 10};
    int tam = 0;
    for (int i = 0; i < 4; i++) {
        cfg.quality = qualidades[i];
        jpeg_enc_handle_t enc;
        if (jpeg_enc_open(&cfg, &enc) != JPEG_ERR_OK) {
            ESP_LOGE(TAG, "sem memória para o codificador");
            return;
        }
        int64_t t0 = esp_timer_get_time();
        jpeg_enc_process(enc, (uint8_t *)px, LARG * ALT * 2,
                         jpg, 48 * 1024, &tam);
        int64_t us = esp_timer_get_time() - t0;
        jpeg_enc_close(enc);
        ESP_LOGI(TAG, "qualidade %2d: %6d bytes (%4.1f:1) em %lld ms",
                 qualidades[i], tam, LARG * ALT * 2.0 / tam, us / 1000);
        for (int j = 0; j < tam; j++) {          /* pela serial, para ver no PC */
            if (j % 48 == 0) {
                printf("\nJPG%d:", qualidades[i]);
            }
            printf("%02x", jpg[j]);
        }
        printf("\n");
    }
    free(jpg);
    free(px);
}
