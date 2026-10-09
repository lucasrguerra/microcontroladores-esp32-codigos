#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/dedic_gpio.h"
#include "hal/dedic_gpio_cpu_ll.h"
#include "esp_cpu.h"
#include "esp_attr.h"

#define PINO   GPIO_NUM_5          /* ponta do osciloscópio aqui */
#define VEZES  1000

static void IRAM_ATTR com_driver(void)
{
    for (int i = 0; i < VEZES; i++) {
        gpio_set_level(PINO, 1);
        gpio_set_level(PINO, 0);
    }
}

static void IRAM_ATTR com_bundle(dedic_gpio_bundle_handle_t b)
{
    for (int i = 0; i < VEZES; i++) {
        dedic_gpio_bundle_write(b, 1, 1);
        dedic_gpio_bundle_write(b, 1, 0);
    }
}

static void IRAM_ATTR com_instrucao(uint32_t mascara)
{
    for (int i = 0; i < VEZES; i++) {
        dedic_gpio_cpu_ll_write_mask(mascara, mascara);
        dedic_gpio_cpu_ll_write_mask(mascara, 0);
    }
}

static void medir(const char *nome, uint32_t ciclos)
{
    printf("%-26s %6lu ciclos por pulso\n", nome, (unsigned long)(ciclos / VEZES));
}

void app_main(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << PINO,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    uint32_t t0 = esp_cpu_get_cycle_count();
    com_driver();
    medir("gpio_set_level()", esp_cpu_get_cycle_count() - t0);

    const int pinos[] = { PINO };
    dedic_gpio_bundle_config_t bcfg = {
        .gpio_array = pinos,
        .array_size = 1,
        .flags.out_en = 1,
    };
    dedic_gpio_bundle_handle_t bundle;
    ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bcfg, &bundle));

    t0 = esp_cpu_get_cycle_count();
    com_bundle(bundle);
    medir("dedic_gpio_bundle_write()", esp_cpu_get_cycle_count() - t0);

    uint32_t mascara, deslocamento;
    ESP_ERROR_CHECK(dedic_gpio_get_out_mask(bundle, &mascara));
    ESP_ERROR_CHECK(dedic_gpio_get_out_offset(bundle, &deslocamento));
    t0 = esp_cpu_get_cycle_count();
    com_instrucao(mascara);
    medir("instrução da CPU", esp_cpu_get_cycle_count() - t0);
    printf("canal dedicado %lu, máscara 0x%02lx\n",
           (unsigned long)deslocamento, (unsigned long)mascara);
}
