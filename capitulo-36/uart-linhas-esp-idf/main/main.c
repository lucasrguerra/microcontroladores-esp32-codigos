#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"

#define PORTA    UART_NUM_1
#define TX       4                  /* ligue um fio do GPIO 4... */
#define RX       5                  /* ...ao GPIO 5 */
#define BUF      1024

static const char *TAG = "linhas";
static QueueHandle_t s_eventos;

static void receber(void *arg)
{
    uart_event_t ev;
    char linha[128];
    for (;;) {
        if (!xQueueReceive(s_eventos, &ev, portMAX_DELAY)) {
            continue;
        }
        switch (ev.type) {
        case UART_PATTERN_DET: {
            int pos = uart_pattern_pop_pos(PORTA);  /* onde está o '\n' */
            if (pos < 0 || pos >= (int)sizeof(linha)) {
                uart_flush_input(PORTA);            /* linha grande demais */
                break;
            }
            int n = uart_read_bytes(PORTA, linha, pos + 1, pdMS_TO_TICKS(100));
            linha[n > 0 ? n - 1 : 0] = '\0';        /* troca o '\n' pelo fim */
            ESP_LOGI(TAG, "linha: \"%s\"", linha);
            break;
        }
        case UART_FIFO_OVF:
        case UART_BUFFER_FULL:
            ESP_LOGW(TAG, "bytes perdidos (evento %d): esvaziando", ev.type);
            uart_flush_input(PORTA);
            xQueueReset(s_eventos);
            break;
        case UART_FRAME_ERR:
            ESP_LOGW(TAG, "erro de quadro: baud rate ou fiação?");
            break;
        default:
            break;
        }
    }
}

void app_main(void)
{
    uart_config_t cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(PORTA, BUF, BUF, 20, &s_eventos, 0));
    ESP_ERROR_CHECK(uart_param_config(PORTA, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(PORTA, TX, RX, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    /* um '\n' sozinho já é o padrão; tempos em ciclos de bit */
    ESP_ERROR_CHECK(uart_enable_pattern_det_baud_intr(PORTA, '\n', 1, 9, 0, 0));
    ESP_ERROR_CHECK(uart_pattern_queue_reset(PORTA, 20));
    xTaskCreate(receber, "receber", 4096, NULL, 10, NULL);

    for (int i = 1; ; i++) {
        char msg[48];
        int n = snprintf(msg, sizeof(msg), "mensagem %d enviada pela UART1\n", i);
        uart_write_bytes(PORTA, msg, n);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
