#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_heap_trace.h"

#define REGISTROS 64
static heap_trace_record_t s_registros[REGISTROS];

/* Monta uma mensagem. No caminho de erro, esquece de liberar o buffer. */
static char *montar(int n)
{
    char *buf = malloc(48);
    if (buf == NULL) {
        return NULL;
    }
    if (n % 5 == 0) {                   /* falha de validação, 1 em cada 5 */
        return NULL;                    /* vazamento: buf não é liberado */
    }
    snprintf(buf, 48, "mensagem %d", n);
    return buf;
}

void app_main(void)
{
    ESP_ERROR_CHECK(heap_trace_init_standalone(s_registros, REGISTROS));
    size_t antes = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);

    ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
    for (int n = 1; n <= 20; n++) {
        char *m = montar(n);
        free(m);                        /* free(NULL) não faz nada */
    }
    ESP_ERROR_CHECK(heap_trace_stop());

    printf("heap livre: antes %u, depois %u bytes\n", (unsigned)antes,
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
    heap_trace_dump();
}
