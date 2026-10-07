#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

// O botão BOOT das placas oficiais fica no GPIO 0 nas séries Xtensa e no GPIO 9 em várias RISC-V.
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define PINO_BOTAO GPIO_NUM_0
#else
#define PINO_BOTAO GPIO_NUM_9
#endif

static const char *TAG = "isr";
static QueueHandle_t fila;

static void IRAM_ATTR isr_botao(void *arg)
{
    int64_t agora = esp_timer_get_time();        // microssegundos desde o boot
    BaseType_t acordou_tarefa = pdFALSE;
    xQueueSendFromISR(fila, &agora, &acordou_tarefa);
    if (acordou_tarefa) {
        portYIELD_FROM_ISR();                    // troca já para a tarefa que espera
    }
}

void app_main(void)
{
    fila = xQueueCreate(10, sizeof(int64_t));

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << PINO_BOTAO,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,      // botão liga o pino ao GND
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,       // interrompe na borda de descida
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(PINO_BOTAO, isr_botao, NULL));

    ESP_LOGI(TAG, "Pressione o botão no GPIO %d", PINO_BOTAO);
    int64_t instante;
    while (1) {
        if (xQueueReceive(fila, &instante, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Interrupção em t = %lld us", instante);
        }
    }
}
