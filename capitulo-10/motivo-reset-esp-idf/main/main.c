#include <stdio.h>
#include "esp_system.h"
#include "esp_attr.h"
#include "esp_log.h"

static const char *TAG = "reset";
static __NOINIT_ATTR uint32_t falhas;   // sobrevive a reinícios, não à falta de energia

static const char *descricao(esp_reset_reason_t r)
{
    switch (r) {
    case ESP_RST_POWERON:   return "energização";
    case ESP_RST_EXT:       return "pino de enable (reset externo)";
    case ESP_RST_SW:        return "esp_restart() chamado pelo software";
    case ESP_RST_PANIC:     return "exceção ou abort()";
    case ESP_RST_INT_WDT:   return "watchdog de interrupção";
    case ESP_RST_TASK_WDT:  return "watchdog de tarefas";
    case ESP_RST_WDT:       return "outro watchdog";
    case ESP_RST_DEEPSLEEP: return "saída do deep sleep";
    case ESP_RST_BROWNOUT:  return "queda de tensão (brownout)";
    case ESP_RST_SDIO:      return "reset pelo SDIO";
    default:                return "outra causa";
    }
}

void app_main(void)
{
    esp_reset_reason_t motivo = esp_reset_reason();
    if (motivo == ESP_RST_POWERON || motivo == ESP_RST_BROWNOUT) {
        falhas = 0;                        // conteúdo indefinido após falta de energia
    } else if (motivo == ESP_RST_PANIC || motivo == ESP_RST_INT_WDT ||
               motivo == ESP_RST_TASK_WDT || motivo == ESP_RST_WDT) {
        falhas++;
    }
    ESP_LOGI(TAG, "Reiniciei por: %s (código %d)", descricao(motivo), motivo);
    ESP_LOGI(TAG, "Falhas desde a última energização: %lu", (unsigned long)falhas);
}
