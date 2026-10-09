#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "sensores";

/* Lê um canal. O sensor do canal 3 não responde. */
static esp_err_t ler_sensor(int canal, int *valor)
{
    ESP_RETURN_ON_FALSE(canal >= 0 && canal < 4, ESP_ERR_INVALID_ARG,
                        TAG, "canal %d não existe", canal);
    if (canal == 3) {
        return ESP_ERR_TIMEOUT;
    }
    *valor = 100 + canal;
    return ESP_OK;
}

/* Lê n canais num vetor novo. Em qualquer erro, libera o vetor e repassa o código. */
static esp_err_t ler_todos(int n, int **saida)
{
    esp_err_t ret = ESP_OK;
    int *v = calloc(n, sizeof(int));
    ESP_RETURN_ON_FALSE(v, ESP_ERR_NO_MEM, TAG, "sem memória para %d canais", n);
    for (int i = 0; i < n; i++) {
        ESP_GOTO_ON_ERROR(ler_sensor(i, &v[i]), falha, TAG, "falha no canal %d", i);
    }
    *saida = v;
    return ESP_OK;
falha:
    free(v);
    return ret;
}

void app_main(void)
{
    int *v = NULL;
    esp_err_t err = ler_todos(3, &v);
    printf("3 canais: %s (%d, %d, %d)\n", esp_err_to_name(err), v[0], v[1], v[2]);
    free(v);

    err = ler_todos(4, &v);
    printf("4 canais: %s (0x%x)\n", esp_err_to_name(err), (unsigned)err);

    esp_log_level_set(TAG, ESP_LOG_NONE);       /* cala só este módulo */
    err = ler_todos(4, &v);
    printf("4 canais, sem log: %s\n", esp_err_to_name(err));
    esp_log_level_set(TAG, ESP_LOG_INFO);

    int x;
    ESP_ERROR_CHECK_WITHOUT_ABORT(ler_sensor(7, &x));
    ESP_ERROR_CHECK(ler_sensor(3, &x));
    printf("esta linha não é impressa\n");
}
