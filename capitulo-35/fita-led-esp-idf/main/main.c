#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "soc/soc_caps.h"

#define PINO        4
#define LEDS        24
#define RESOLUCAO   10000000                /* 10 MHz: 0,1 µs por tique */

static uint8_t s_grb[LEDS * 3];

static const rmt_symbol_word_t BIT_0 = { .level0 = 1, .duration0 = 3,
                                         .level1 = 0, .duration1 = 9 };
static const rmt_symbol_word_t BIT_1 = { .level0 = 1, .duration0 = 9,
                                         .level1 = 0, .duration1 = 3 };
static const rmt_symbol_word_t RESET = { .level0 = 0, .duration0 = 250,
                                         .level1 = 0, .duration1 = 250 };

/* Chamada pelo driver sempre que há espaço na memória do canal. */
static size_t codificar(const void *dados, size_t tam, size_t escritos, size_t livres,
                        rmt_symbol_word_t *simb, bool *fim, void *arg)
{
    if (livres < 8) {
        return 0;                           /* sem espaço para um byte inteiro */
    }
    size_t byte = escritos / 8;
    if (byte < tam) {
        uint8_t v = ((const uint8_t *)dados)[byte];
        for (int i = 0; i < 8; i++) {
            simb[i] = (v & (0x80 >> i)) ? BIT_1 : BIT_0;
        }
        return 8;
    }
    simb[0] = RESET;                        /* 50 µs em nível baixo */
    *fim = true;
    return 1;
}

void app_main(void)
{
    rmt_channel_handle_t canal;
    rmt_tx_channel_config_t cfg = {
        .gpio_num = PINO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUCAO,
        .mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL,
        .trans_queue_depth = 2,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&cfg, &canal));

    rmt_encoder_handle_t enc;
    rmt_simple_encoder_config_t ecfg = { .callback = codificar };
    ESP_ERROR_CHECK(rmt_new_simple_encoder(&ecfg, &enc));
    ESP_ERROR_CHECK(rmt_enable(canal));

    rmt_transmit_config_t tcfg = { .loop_count = 0 };
    for (int pos = 0; ; pos = (pos + 1) % LEDS) {
        memset(s_grb, 0, sizeof(s_grb));
        for (int k = 0; k < 4; k++) {       /* cabeça e cauda que se apaga */
            int led = (pos - k + LEDS) % LEDS;
            s_grb[led * 3 + 1] = 200 >> (2 * k);   /* byte do vermelho */
        }
        ESP_ERROR_CHECK(rmt_transmit(canal, enc, s_grb, sizeof(s_grb), &tcfg));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(canal, portMAX_DELAY));
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}
