#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_pdm.h"

#if CONFIG_IDF_TARGET_ESP32
#define CLK  22
#define DIN  21
#else
#define CLK  4
#define DIN  5
#endif
#define TAXA 16000
#define N    1600                         /* 100 ms */

static int16_t s_pcm[N];

void app_main(void)
{
    i2s_chan_handle_t rx;
    i2s_chan_config_t ccfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&ccfg, NULL, &rx));

    i2s_pdm_rx_config_t pdm = {
        .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(TAXA),
        .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                   I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = CLK,
            .din = DIN,
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx, &pdm));
    ESP_ERROR_CHECK(i2s_channel_enable(rx));

    for (;;) {
        size_t bytes = 0;
        ESP_ERROR_CHECK(i2s_channel_read(rx, s_pcm, sizeof(s_pcm), &bytes, 1000));
        size_t n = bytes / sizeof(int16_t);
        double soma = 0;
        for (size_t i = 0; i < n; i++) {
            double v = s_pcm[i] / 32768.0;
            soma += v * v;
        }
        double dbfs = n ? 20.0 * log10(sqrt(soma / n) + 1e-9) : -200.0;
        printf("%u amostras, nível %.1f dBFS\n", (unsigned)n, dbfs);
    }
}
