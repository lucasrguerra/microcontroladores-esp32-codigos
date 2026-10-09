#include <stdio.h>
#include <stdlib.h>
#include "esp_heap_caps.h"

#define BLOCO 2048
#define MAX_BLOCOS 256

static void *s_blocos[MAX_BLOCOS];

static void mostrar(const char *quando)
{
    printf("%-26s livre %6u   maior bloco %6u\n", quando,
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
           (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
}

void app_main(void)
{
    mostrar("no início:");

    int n = 0;
    while (n < MAX_BLOCOS && (s_blocos[n] = malloc(BLOCO)) != NULL) {
        n++;                            /* enche o heap de blocos de 2 KB */
    }
    for (int i = 0; i < n; i += 2) {
        free(s_blocos[i]);              /* devolve um sim, um não */
        s_blocos[i] = NULL;
    }
    printf("%d blocos alocados, metade liberada\n", n);
    mostrar("com buracos:");

    void *grande = malloc(8192);
    printf("malloc(8192): %s\n", grande ? "conseguiu" : "falhou");
    free(grande);

    for (int i = 1; i < n; i += 2) {
        free(s_blocos[i]);
    }
    mostrar("tudo liberado:");
    printf("mínimo desde o boot: %u bytes\n",
           (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT));
}
