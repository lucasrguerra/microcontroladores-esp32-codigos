#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

static const char *TAG = "base";

/* A mesma soma dos Caps. 75 e 76. O volatile impede o compilador de
   calcular o resultado em tempo de compilação. */
static uint32_t soma(uint32_t n)
{
    volatile uint32_t limite = n;
    uint32_t s = 0;
    for (uint32_t i = 0; i < limite; i++) {
        s += i & 0xFF;
    }
    return s;
}

void app_main(void)
{
    int64_t t0 = esp_timer_get_time();
    uint32_t r = soma(100000);
    int64_t us = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "soma: resultado %" PRIu32 " em %lld us", r, us);
    ESP_LOGI(TAG, "heap livre: %u bytes (maior bloco %u)",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}
