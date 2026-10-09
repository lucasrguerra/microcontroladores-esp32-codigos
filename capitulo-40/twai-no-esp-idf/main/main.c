#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "esp_log.h"

#define PINO_TX  4                          /* ao TXD/D do transceptor */
#define PINO_RX  5                          /* ao RXD/R do transceptor */
#define MEU_ID   0x101

static const char *TAG = "no_can";
static QueueHandle_t s_fila;
static TaskHandle_t s_tarefa;
static const char *ESTADO[] = { "error active", "error warning",
                                "error passive", "bus off" };

typedef struct { uint32_t id; uint8_t n; uint8_t dados[8]; } msg_t;

static bool IRAM_ATTR chegou(twai_node_handle_t h,
                             const twai_rx_done_event_data_t *ev, void *arg)
{
    msg_t m;
    twai_frame_t q = { .buffer = m.dados, .buffer_len = sizeof(m.dados) };
    BaseType_t acordou = pdFALSE;
    if (twai_node_receive_from_isr(h, &q) == ESP_OK) {
        m.id = q.header.id;
        m.n = q.header.dlc;
        xQueueSendFromISR(s_fila, &m, &acordou);
    }
    return acordou == pdTRUE;
}

static bool IRAM_ATTR mudou(twai_node_handle_t h,
                            const twai_state_change_event_data_t *ev, void *arg)
{
    BaseType_t acordou = pdFALSE;
    ESP_EARLY_LOGW(TAG, "estado: %s -> %s", ESTADO[ev->old_sta], ESTADO[ev->new_sta]);
    if (ev->new_sta == TWAI_ERROR_BUS_OFF) {
        vTaskNotifyGiveFromISR(s_tarefa, &acordou);   /* recuperar numa tarefa */
    }
    return acordou == pdTRUE;
}

static void recuperar(void *arg)
{
    twai_node_handle_t no = arg;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));           /* dá tempo de o defeito sumir */
        ESP_LOGW(TAG, "tentando sair do bus-off");
        twai_node_recover(no);
    }
}

void app_main(void)
{
    s_fila = xQueueCreate(16, sizeof(msg_t));
    twai_node_handle_t no;
    twai_onchip_node_config_t cfg = {
        .io_cfg = {
            .tx = PINO_TX,
            .rx = PINO_RX,
            .quanta_clk_out = GPIO_NUM_NC,
            .bus_off_indicator = GPIO_NUM_NC,
        },
        .bit_timing.bitrate = 250000,
        .tx_queue_depth = 8,
    };
    ESP_ERROR_CHECK(twai_new_node_onchip(&cfg, &no));

    twai_mask_filter_config_t filtro = {
        .id = 0x200,
        .mask = 0x7F0,                      /* compara os 7 bits de cima */
    };
    ESP_ERROR_CHECK(twai_node_config_mask_filter(no, 0, &filtro));

    xTaskCreate(recuperar, "recuperar", 3072, no, 6, &s_tarefa);
    twai_event_callbacks_t cbs = { .on_rx_done = chegou, .on_state_change = mudou };
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(no, &cbs, NULL));
    ESP_ERROR_CHECK(twai_node_enable(no));

    uint32_t contador = 0;
    for (;;) {
        uint8_t carga[4] = { (uint8_t)(contador >> 24), (uint8_t)(contador >> 16),
                             (uint8_t)(contador >> 8), (uint8_t)contador };
        twai_frame_t q = {
            .header = { .id = MEU_ID, .dlc = 4 },
            .buffer = carga,
            .buffer_len = sizeof(carga),
        };
        if (twai_node_transmit(no, &q, 0) != ESP_OK) {
            ESP_LOGW(TAG, "fila de transmissão cheia");
        }
        contador++;

        msg_t m;
        while (xQueueReceive(s_fila, &m, pdMS_TO_TICKS(1000)) == pdTRUE) {
            printf("recebi 0x%03lX com %u bytes:", (unsigned long)m.id, m.n);
            for (int i = 0; i < m.n && i < 8; i++) printf(" %02X", m.dados[i]);
            printf("\n");
        }
        twai_node_status_t st;
        twai_node_get_info(no, &st, NULL);
        ESP_LOGI(TAG, "TEC %u, REC %u", st.tx_error_count, st.rx_error_count);
    }
}
