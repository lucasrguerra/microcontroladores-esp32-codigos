#include <stdio.h>
#include <inttypes.h>
#include "esp_cpu.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define N 10000
static const char *TAG = "fpu";
static volatile uint32_t resultado_i;     // volatile: os resultados precisam
static volatile float    resultado_f;     // "sair" do laço, senão o compilador
static volatile double   resultado_d;     // elimina as contas

void app_main(void)
{
    uint32_t ai = 3;
    float    af = 1.0001f;
    double   ad = 1.0001;

    uint32_t t0 = esp_cpu_get_cycle_count();
    for (int i = 0; i < N; i++) {
        ai = ai * 3u + (uint32_t)i;
    }
    uint32_t t1 = esp_cpu_get_cycle_count();
    for (int i = 0; i < N; i++) {
        af = af * 1.0001f + 0.5f;
    }
    uint32_t t2 = esp_cpu_get_cycle_count();
    for (int i = 0; i < N; i++) {
        ad = ad * 1.0001 + 0.5;
    }
    uint32_t t3 = esp_cpu_get_cycle_count();
    resultado_i = ai;
    resultado_f = af;
    resultado_d = ad;

    ESP_LOGI(TAG, "%s a %d MHz", CONFIG_IDF_TARGET, CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
    ESP_LOGI(TAG, "uint32: %6.1f ciclos por iteração", (t1 - t0) / (double)N);
    ESP_LOGI(TAG, "float : %6.1f ciclos por iteração", (t2 - t1) / (double)N);
    ESP_LOGI(TAG, "double: %6.1f ciclos por iteração", (t3 - t2) / (double)N);
}
