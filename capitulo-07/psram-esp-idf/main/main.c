#include <stdio.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAMANHO (256 * 1024)
static const char *TAG = "psram";

static int64_t preencher(uint32_t *buf, size_t palavras)
{
    int64_t t0 = esp_timer_get_time();
    for (size_t i = 0; i < palavras; i++) {
        buf[i] = (uint32_t)i;
    }
    return esp_timer_get_time() - t0;
}

void app_main(void)
{
    size_t n = TAMANHO / sizeof(uint32_t);
    uint32_t *interno = heap_caps_malloc(TAMANHO, MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    uint32_t *externo = heap_caps_malloc(TAMANHO, MALLOC_CAP_SPIRAM);
    if (!externo) {
        ESP_LOGE(TAG, "Sem PSRAM: confira a placa e o CONFIG_SPIRAM");
        return;
    }
    if (interno) {
        ESP_LOGI(TAG, "SRAM interna: %lld us", preencher(interno, n));
        heap_caps_free(interno);
    } else {
        ESP_LOGW(TAG, "Não há 256 KB contíguos na SRAM interna");
    }
    ESP_LOGI(TAG, "PSRAM:        %lld us", preencher(externo, n));
    heap_caps_free(externo);
}
