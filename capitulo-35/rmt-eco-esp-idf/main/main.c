#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "soc/soc_caps.h"

#define SAIDA      4                        /* ligue um fio do GPIO 4... */
#define ENTRADA    5                        /* ...ao GPIO 5 */
#define RESOLUCAO  1000000                  /* 1 MHz: 1 tique = 1 µs */
#define N          SOC_RMT_MEM_WORDS_PER_CHANNEL

static QueueHandle_t s_fila;
static rmt_symbol_word_t s_recebido[N];

static bool IRAM_ATTR recebeu(rmt_channel_handle_t c,
                              const rmt_rx_done_event_data_t *ev, void *arg)
{
    BaseType_t acordou = pdFALSE;
    xQueueSendFromISR(s_fila, &ev->num_symbols, &acordou);
    return acordou == pdTRUE;
}

void app_main(void)
{
    s_fila = xQueueCreate(1, sizeof(size_t));

    rmt_channel_handle_t rx;
    rmt_rx_channel_config_t rcfg = {
        .gpio_num = ENTRADA,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUCAO,
        .mem_block_symbols = N,
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rcfg, &rx));
    rmt_rx_event_callbacks_t cbs = { .on_recv_done = recebeu };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx, &cbs, NULL));
    ESP_ERROR_CHECK(rmt_enable(rx));

    rmt_channel_handle_t tx;
    rmt_tx_channel_config_t tcfg = {
        .gpio_num = SAIDA,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUCAO,
        .mem_block_symbols = N,
        .trans_queue_depth = 1,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tcfg, &tx));
    rmt_encoder_handle_t copia;
    rmt_copy_encoder_config_t ccfg = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&ccfg, &copia));
    ESP_ERROR_CHECK(rmt_enable(tx));

    /* Quatro pulsos de 100, 200, 300 e 400 µs, separados por 150 µs. */
    rmt_symbol_word_t teste[4];
    for (int i = 0; i < 4; i++) {
        teste[i] = (rmt_symbol_word_t){ .level0 = 1, .duration0 = 100 * (i + 1),
                                        .level1 = 0, .duration1 = 150 };
    }
    rmt_receive_config_t rx_cfg = {
        .signal_range_min_ns = 2000,        /* abaixo de 2 µs é ruído */
        .signal_range_max_ns = 1000000,     /* 1 ms parado = fim da mensagem */
    };
    rmt_transmit_config_t envio = { .loop_count = 0 };

    for (;;) {
        ESP_ERROR_CHECK(rmt_receive(rx, s_recebido, sizeof(s_recebido), &rx_cfg));
        ESP_ERROR_CHECK(rmt_transmit(tx, copia, teste, sizeof(teste), &envio));
        size_t n;
        if (xQueueReceive(s_fila, &n, pdMS_TO_TICKS(100)) == pdTRUE) {
            printf("recebi %u símbolos:", (unsigned)n);
            for (size_t i = 0; i < n; i++) {
                printf(" [%u µs em %u, %u µs em %u]",
                       s_recebido[i].duration0, s_recebido[i].level0,
                       s_recebido[i].duration1, s_recebido[i].level1);
            }
            printf("\n");
        } else {
            printf("nada recebido: confira o fio entre os GPIO %d e %d\n",
                   SAIDA, ENTRADA);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
