#include <stdio.h>
#include <stdbool.h>
#include "cJSON.h"

static const char *CONFIG =
    "{\"nome\":\"estufa-3\",\"intervalo_s\":60,"
    "\"limites\":{\"min\":18.5,\"max\":27}}";

void app_main(void)
{
    cJSON *raiz = cJSON_Parse(CONFIG);
    if (raiz == NULL) {
        printf("JSON inválido perto de: %s\n", cJSON_GetErrorPtr());
        return;
    }
    const cJSON *nome = cJSON_GetObjectItem(raiz, "nome");
    const cJSON *intervalo = cJSON_GetObjectItem(raiz, "intervalo_s");
    const cJSON *limites = cJSON_GetObjectItem(raiz, "limites");
    const cJSON *max = cJSON_GetObjectItem(limites, "max");
    if (cJSON_IsString(nome) && cJSON_IsNumber(intervalo) && cJSON_IsNumber(max)) {
        printf("%s: mede a cada %d s, alarme acima de %.1f\n",
               nome->valuestring, intervalo->valueint, max->valuedouble);
    }

    cJSON_AddBoolToObject(raiz, "ativo", true);
    char *texto = cJSON_PrintUnformatted(raiz);
    printf("%s\n", texto);
    cJSON_free(texto);
    cJSON_Delete(raiz);
}
