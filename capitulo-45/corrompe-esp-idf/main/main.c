#include <stdio.h>
#include <stdlib.h>
#include "esp_heap_caps.h"

/* Cópia sem limite, como a de um protocolo mal escrito. */
static void __attribute__((noinline)) copiar(char *destino, const char *origem)
{
    while ((*destino++ = *origem++) != '\0') {
    }
}

void app_main(void)
{
    char *nome = malloc(8);
    copiar(nome, "ESP32-S3");           /* 8 letras + terminador = 9 bytes */
    printf("gravado: %s\n", nome);

    if (!heap_caps_check_integrity_all(true)) {
        printf("o heap já está corrompido, e nada parou ainda\n");
    }
    free(nome);
    printf("esta linha não é impressa\n");
}
