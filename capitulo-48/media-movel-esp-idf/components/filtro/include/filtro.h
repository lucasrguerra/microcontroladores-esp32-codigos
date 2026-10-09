#pragma once
#include <stdint.h>

typedef struct {
    int32_t amostras[CONFIG_FILTRO_JANELA];
    int32_t soma;
    int n;              /* amostras válidas, até a janela */
    int pos;            /* próxima posição a sobrescrever */
} filtro_t;

void filtro_iniciar(filtro_t *f);
int32_t filtro_aplicar(filtro_t *f, int32_t amostra);
