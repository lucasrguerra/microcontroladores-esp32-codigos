#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_err.h"
#include "esp_timer.h"
#include "driver/jpeg_encode.h"

#define LARGURA 640
#define ALTURA  480
#define TAMANHO (LARGURA * ALTURA * 3)

void app_main(void)
{
    size_t tam_entrada = 0, tam_saida = 0;
    jpeg_encode_memory_alloc_cfg_t cfg_in = {.buffer_direction = JPEG_ENC_ALLOC_INPUT_BUFFER};
    jpeg_encode_memory_alloc_cfg_t cfg_out = {.buffer_direction = JPEG_ENC_ALLOC_OUTPUT_BUFFER};
    uint8_t *rgb = jpeg_alloc_encoder_mem(TAMANHO, &cfg_in, &tam_entrada);
    uint8_t *jpg = jpeg_alloc_encoder_mem(TAMANHO / 10, &cfg_out, &tam_saida);
    if (rgb == NULL || jpg == NULL) {
        printf("Sem memória: confira se a PSRAM está ligada.\n");
        return;
    }

    for (int y = 0; y < ALTURA; y++) {          // degradê horizontal e vertical
        for (int x = 0; x < LARGURA; x++) {
            uint8_t *p = &rgb[(y * LARGURA + x) * 3];
            p[0] = x * 255 / LARGURA;          // azul (ordem BGR)
            p[1] = y * 255 / ALTURA;           // verde
            p[2] = 128;                        // vermelho
        }
    }

    jpeg_encoder_handle_t cod = NULL;
    jpeg_encode_engine_cfg_t cfg_eng = {.timeout_ms = 100};
    ESP_ERROR_CHECK(jpeg_new_encoder_engine(&cfg_eng, &cod));
    jpeg_encode_cfg_t cfg = {
        .src_type = JPEG_ENCODE_IN_FORMAT_RGB888,
        .sub_sample = JPEG_DOWN_SAMPLING_YUV420,
        .image_quality = 80,
        .width = LARGURA,
        .height = ALTURA,
    };

    uint32_t tam_jpg = 0;
    int64_t t0 = esp_timer_get_time();
    ESP_ERROR_CHECK(jpeg_encoder_process(cod, &cfg, rgb, TAMANHO, jpg, tam_saida, &tam_jpg));
    int64_t t1 = esp_timer_get_time();

    printf("%dx%d: %d bytes -> %" PRIu32 " bytes em %lld us\n",
           LARGURA, ALTURA, TAMANHO, tam_jpg, (long long)(t1 - t0));

    ESP_ERROR_CHECK(jpeg_del_encoder_engine(cod));
    free(rgb);
    free(jpg);
}
