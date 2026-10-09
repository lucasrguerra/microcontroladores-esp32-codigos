#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"

#if CONFIG_IDF_TARGET_ESP32
#define BCLK 26
#define WS   25
#define DOUT 22
#else
#define BCLK 4
#define WS   5
#define DOUT 6
#endif
#define TAXA     44100
#define FREQ     440.0f
#define QUADROS  441                      /* 10 ms de áudio por bloco */

static int16_t s_bloco[QUADROS * 2];      /* esquerdo e direito intercalados */

void app_main(void)
{
    i2s_chan_handle_t tx;
    i2s_chan_config_t ccfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ccfg.dma_frame_num = 240;
    ESP_ERROR_CHECK(i2s_new_channel(&ccfg, &tx, NULL));

    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(TAXA),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                        I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,      /* o MAX98357A não precisa de MCLK */
            .bclk = BCLK,
            .ws = WS,
            .dout = DOUT,
            .din = I2S_GPIO_UNUSED,
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx, &std));
    ESP_ERROR_CHECK(i2s_channel_enable(tx));

    float fase = 0;
    const float passo = 2.0f * (float)M_PI * FREQ / TAXA;
    for (;;) {
        for (int i = 0; i < QUADROS; i++) {
            int16_t v = (int16_t)(sinf(fase) * 8000);   /* um quarto da escala */
            s_bloco[2 * i] = v;
            s_bloco[2 * i + 1] = v;
            fase += passo;
            if (fase > 2.0f * (float)M_PI) fase -= 2.0f * (float)M_PI;
        }
        size_t escritos;
        ESP_ERROR_CHECK(i2s_channel_write(tx, s_bloco, sizeof(s_bloco), &escritos,
                                          portMAX_DELAY));
    }
}
