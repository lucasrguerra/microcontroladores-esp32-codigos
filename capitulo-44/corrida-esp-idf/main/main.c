#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_rom_sys.h"

#define VOLTAS 5000

typedef struct {
    uint32_t total;
    bool protegido;
} contador_t;

static contador_t s_livre = { 0, false };
static contador_t s_guardado = { 0, true };
static SemaphoreHandle_t s_mutex;
static SemaphoreHandle_t s_fim;

/* Ler, processar e gravar: o padrão de quase todo dado compartilhado. */
static void atualizar(contador_t *c)
{
    if (c->protegido) {
        xSemaphoreTake(s_mutex, portMAX_DELAY);
    }
    uint32_t valor = c->total;          /* lê */
    esp_rom_delay_us(50);               /* processa (filtro, conversão...) */
    c->total = valor + 1;               /* grava */
    if (c->protegido) {
        xSemaphoreGive(s_mutex);
    }
}

static void trabalho(void *arg)
{
    contador_t *c = arg;
    for (int i = 0; i < VOLTAS; i++) {
        atualizar(c);
    }
    xSemaphoreGive(s_fim);
    vTaskDelete(NULL);
}

static void rodar(contador_t *c, const char *nome)
{
    xTaskCreate(trabalho, "a", 2048, c, 5, NULL);
    xTaskCreate(trabalho, "b", 2048, c, 5, NULL);
    xSemaphoreTake(s_fim, portMAX_DELAY);
    xSemaphoreTake(s_fim, portMAX_DELAY);
    printf("%-14s esperado %u, obtido %lu\n", nome, 2 * VOLTAS,
           (unsigned long)c->total);
}

void app_main(void)
{
    vTaskPrioritySet(NULL, 10);         /* cria as duas antes que elas rodem */
    s_mutex = xSemaphoreCreateMutex();
    s_fim = xSemaphoreCreateCounting(2, 0);
    rodar(&s_livre, "sem proteção:");
    rodar(&s_guardado, "com mutex:");
}
