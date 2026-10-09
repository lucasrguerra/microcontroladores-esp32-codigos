#include <stdio.h>
#include "sdkconfig.h"
#include "filtro.h"

void app_main(void)
{
    filtro_t f;
    filtro_iniciar(&f);
    const int32_t leituras[] = { 100, 104, 98, 250, 101, 99, 103, 97, 102, 100 };

    printf("janela de %d amostras\n", CONFIG_FILTRO_JANELA);
    for (int i = 0; i < 10; i++) {
        printf("leitura %3ld -> média %3ld\n", (long)leituras[i],
               (long)filtro_aplicar(&f, leituras[i]));
    }
}
