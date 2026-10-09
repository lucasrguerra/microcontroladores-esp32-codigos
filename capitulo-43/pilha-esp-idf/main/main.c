#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Simula um processamento que usa 'n' bytes de variáveis locais. */
static int processar(size_t n)
{
    volatile uint8_t buffer[n];
    memset((void *)buffer, 0x5A, n);
    return buffer[n - 1];
}

static void trabalho(void *arg)
{
    for (size_t n = 256; ; n += 256) {
        processar(n);
        printf("buffer de %4u bytes: sobram %4u bytes de pilha\n",
               (unsigned)n, (unsigned)uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
{
    xTaskCreate(trabalho, "trabalho", 2048, NULL, 5, NULL);
}
