#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_cpu.h"          // esp_cpu_get_cycle_count()
#include "soc/gpio_reg.h"     // endereços dos registradores de GPIO

#define PINO    GPIO_NUM_4
#define PULSOS  1000
static const char *TAG = "regs";

void app_main(void)
{
    gpio_reset_pin(PINO);
    gpio_set_direction(PINO, GPIO_MODE_OUTPUT);

    // 1) Pela API do driver
    uint32_t t0 = esp_cpu_get_cycle_count();
    for (int i = 0; i < PULSOS; i++) {
        gpio_set_level(PINO, 1);
        gpio_set_level(PINO, 0);
    }
    // 2) Escrevendo direto nos registradores W1TS e W1TC
    uint32_t t1 = esp_cpu_get_cycle_count();
    for (int i = 0; i < PULSOS; i++) {
        REG_WRITE(GPIO_OUT_W1TS_REG, BIT(PINO));
        REG_WRITE(GPIO_OUT_W1TC_REG, BIT(PINO));
    }
    uint32_t t2 = esp_cpu_get_cycle_count();

    ESP_LOGI(TAG, "Driver:      %lu ciclos por pulso", (unsigned long)((t1 - t0) / PULSOS));
    ESP_LOGI(TAG, "Registrador: %lu ciclos por pulso", (unsigned long)((t2 - t1) / PULSOS));
    ESP_LOGI(TAG, "GPIO_OUT_W1TS_REG fica no endereço 0x%08lx",
             (unsigned long)GPIO_OUT_W1TS_REG);
}
