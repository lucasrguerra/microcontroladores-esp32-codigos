#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "sdkconfig.h"

static const char *TAG = "pane";

#if CONFIG_PANE_PONTEIRO_NULO
static void gravar_leitura(uint32_t *destino, uint32_t valor)
{
    *destino = valor;               /* destino NULL: a falha é aqui */
}

static void provocar(void)
{
    uint32_t *sensor = NULL;        /* deveria apontar para um buffer */
    gravar_leitura(sensor, 42);
}

#elif CONFIG_PANE_ESTOURO_PILHA
static int consumir(int n)
{
    volatile char local[256];       /* cada nível ocupa mais 256 bytes */
    memset((char *)local, n, sizeof(local));
    return n > 0 ? consumir(n - 1) + local[0] : local[0];
}

static void tarefa_pequena(void *arg)
{
    int r = consumir(12);           /* ~3 KB numa pilha de 2 KB */
    vTaskDelay(1);                  /* a troca de contexto confere a pilha */
    ESP_LOGI(TAG, "não deveria chegar aqui (%d)", r);
    vTaskDelete(NULL);
}

static void provocar(void)
{
    xTaskCreate(tarefa_pequena, "pequena", 2048, NULL, 5, NULL);
}

#elif CONFIG_PANE_WATCHDOG
static void provocar(void)
{
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    while (1) {
        /* laço sem vTaskDelay: a tarefa ociosa nunca roda */
    }
}

#elif CONFIG_PANE_ERROR_CHECK
static void provocar(void)
{
    /* apaga o laço de eventos padrão sem tê-lo criado */
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
}
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "A falha vem em 3 s");
    vTaskDelay(pdMS_TO_TICKS(3000));
    provocar();
}
