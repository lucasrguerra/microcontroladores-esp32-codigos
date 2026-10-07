#include <stdio.h>
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "memoria";

static void mostrar(const char *nome, uint32_t caps)
{
    size_t livre  = heap_caps_get_free_size(caps);
    size_t bloco  = heap_caps_get_largest_free_block(caps);
    size_t minimo = heap_caps_get_minimum_free_size(caps);
    ESP_LOGI(TAG, "%-10s livre %7u B | maior bloco %7u B | mínimo histórico %7u B",
             nome, (unsigned)livre, (unsigned)bloco, (unsigned)minimo);
}

void app_main(void)
{
    mostrar("interna", MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    mostrar("DMA",     MALLOC_CAP_DMA);
    mostrar("PSRAM",   MALLOC_CAP_SPIRAM);
    mostrar("IRAM",    MALLOC_CAP_EXEC);
    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);   // detalhe por região
}
