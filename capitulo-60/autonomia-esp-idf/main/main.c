#include <stdio.h>

/* Uma fase do ciclo: corrente média e duração. Troque pelos valores MEDIDOS. */
typedef struct {
    const char *nome;
    double ma;       /* corrente média na fase, mA */
    double ms;       /* duração da fase, ms */
} fase_t;

/* Ciclo de um sensor ESP32-C3 que acorda, mede e envia por ESP-NOW. As correntes
 * de rádio e de sono são do datasheet; as durações são estimativas a medir. */
static const fase_t ciclo[] = {
    {"boot e inicialização", 25.0, 150.0},
    {"leitura do sensor",    25.0,  20.0},
    {"rádio transmitindo",  335.0,   5.0},
    {"rádio esperando ACK",  84.0,  20.0},
};
#define N_FASES (sizeof(ciclo) / sizeof(ciclo[0]))

static const double SONO_UA = 5.0;        /* deep sleep do C3, datasheet */
static const double BATERIA_MAH = 2500;   /* uma célula 18650, valor nominal */
static const double APROVEITA = 0.8;      /* margem: temperatura, envelhecimento */

static double media_ma(double periodo_s, double extra_ua)
{
    double carga_mas = 0, ativo_ms = 0;   /* mA·s gastos acordado */
    for (int i = 0; i < N_FASES; i++) {
        carga_mas += ciclo[i].ma * ciclo[i].ms / 1000.0;
        ativo_ms += ciclo[i].ms;
    }
    double sono_s = periodo_s - ativo_ms / 1000.0;
    double sono_mas = (SONO_UA + extra_ua) / 1000.0 * sono_s;
    double extra_ativo = extra_ua / 1000.0 * ativo_ms / 1000.0;
    return (carga_mas + sono_mas + extra_ativo) / periodo_s;
}

static void tabela(const char *titulo, double extra_ua)
{
    static const double periodos[] = {10, 60, 300, 900, 3600};
    printf("\n%s\n", titulo);
    printf("  intervalo   média (µA)   autonomia (dias)\n");
    for (int i = 0; i < 5; i++) {
        double ma = media_ma(periodos[i], extra_ua);
        double horas = BATERIA_MAH * APROVEITA / ma;
        printf("  %6.0f s   %10.1f   %14.0f\n", periodos[i], ma * 1000, horas / 24);
    }
}

void app_main(void)
{
    double carga = 0;
    for (int i = 0; i < N_FASES; i++) {
        carga += ciclo[i].ma * ciclo[i].ms / 1000.0;
    }
    printf("carga por despertar: %.2f mA·s\n", carga);
    tabela("só o chip (5 µA em deep sleep):", 0);
    tabela("com regulador de 5 mA quiescente:", 5000);
}
