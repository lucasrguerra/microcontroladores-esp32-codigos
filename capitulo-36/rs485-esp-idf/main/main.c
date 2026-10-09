#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

#define PORTA  UART_NUM_1
#define TX     4                    /* ao DI do transceptor */
#define RX     5                    /* ao RO do transceptor */
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
#define RTS    18                   /* ao DE e ao RE, ligados juntos */
#else
#define RTS    6                    /* no C3, o GPIO 18 é do USB */
#endif

void app_main(void)
{
    uart_config_t cfg = {
        .baud_rate = 19200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,         /* o Modbus RTU usa paridade par */
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(PORTA, 512, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(PORTA, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(PORTA, TX, RX, RTS, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(PORTA, UART_MODE_RS485_HALF_DUPLEX));
    ESP_ERROR_CHECK(uart_set_rx_timeout(PORTA, 3));   /* fim após ~3 caracteres */

    const char pergunta[] = "?\r\n";
    uint8_t resposta[128];
    for (;;) {
        uart_write_bytes(PORTA, pergunta, strlen(pergunta));
        ESP_ERROR_CHECK(uart_wait_tx_done(PORTA, pdMS_TO_TICKS(100)));
        int n = uart_read_bytes(PORTA, resposta, sizeof(resposta) - 1,
                                pdMS_TO_TICKS(200));
        bool colisao = false;
        uart_get_collision_flag(PORTA, &colisao);
        if (n > 0) {
            resposta[n] = '\0';
            printf("resposta (%d bytes)%s: %s\n", n, colisao ? " com colisão" : "",
                   (char *)resposta);
        } else {
            printf("sem resposta no barramento\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
