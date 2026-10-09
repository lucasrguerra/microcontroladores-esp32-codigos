#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"

#define PRONTO_REDE    BIT0
#define PRONTO_SENSOR  BIT1
#define PRONTO_CARTAO  BIT2
#define TODOS (PRONTO_REDE | PRONTO_SENSOR | PRONTO_CARTAO)

typedef struct {
    const char *nome;
    EventBits_t bit;
    int ms;                             /* tempo de inicialização; < 0: falha */
} subsistema_t;

static EventGroupHandle_t s_estado;
static RingbufHandle_t s_registro;

static void registrar(const char *texto)
{
    xRingbufferSend(s_registro, texto, strlen(texto) + 1, pdMS_TO_TICKS(10));
}

static void iniciar(void *arg)
{
    const subsistema_t *s = arg;
    char linha[48];
    vTaskDelay(pdMS_TO_TICKS(s->ms < 0 ? -s->ms : s->ms));
    if (s->ms > 0) {
        xEventGroupSetBits(s_estado, s->bit);
        snprintf(linha, sizeof linha, "%s pronto em %d ms", s->nome, s->ms);
    } else {
        snprintf(linha, sizeof linha, "%s falhou", s->nome);
    }
    registrar(linha);
    vTaskDelete(NULL);
}

static void escritor(void *arg)
{
    for (;;) {
        size_t n;
        char *item = xRingbufferReceive(s_registro, &n, portMAX_DELAY);
        printf("[registro] %s\n", item);
        vRingbufferReturnItem(s_registro, item);
    }
}

static const subsistema_t s_subs[] = {
    { "rede", PRONTO_REDE, 300 },
    { "sensor", PRONTO_SENSOR, 120 },
    { "cartão", PRONTO_CARTAO, -500 },
};

void app_main(void)
{
    s_estado = xEventGroupCreate();
    s_registro = xRingbufferCreate(512, RINGBUF_TYPE_NOSPLIT);
    xTaskCreate(escritor, "escritor", 3072, NULL, 2, NULL);
    for (int i = 0; i < 3; i++) {
        xTaskCreate(iniciar, s_subs[i].nome, 2560, (void *)&s_subs[i], 5, NULL);
    }

    EventBits_t bits = xEventGroupWaitBits(s_estado, TODOS, pdFALSE, pdTRUE,
                                           pdMS_TO_TICKS(1000));
    if ((bits & TODOS) == TODOS) {
        registrar("tudo pronto");
    } else {
        for (int i = 0; i < 3; i++) {
            if (!(bits & s_subs[i].bit)) {
                printf("prazo esgotado: falta %s\n", s_subs[i].nome);
            }
        }
    }
}
