#include <stdio.h>
#include <inttypes.h>
#include "esp_clk_tree.h"
#include "esp_log.h"

static const char *TAG = "clocks";

static void mostrar(const char *nome, soc_module_clk_t fonte)
{
    uint32_t hz = 0;
    esp_clk_tree_src_get_freq_hz(fonte, ESP_CLK_TREE_SRC_FREQ_PRECISION_APPROX, &hz);
    ESP_LOGI(TAG, "%-9s %10" PRIu32 " Hz", nome, hz);
}

void app_main(void)
{
    mostrar("CPU", SOC_MOD_CLK_CPU);
    mostrar("XTAL", SOC_MOD_CLK_XTAL);
    mostrar("RTC_SLOW", SOC_MOD_CLK_RTC_SLOW);
}
