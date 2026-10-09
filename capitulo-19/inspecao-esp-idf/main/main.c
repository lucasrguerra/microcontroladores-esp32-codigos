#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "esp_chip_info.h"
#include "esp_efuse.h"
#include "esp_flash.h"
#include "sdkconfig.h"
#if CONFIG_SPIRAM
#include "esp_psram.h"
#endif

// Copie aqui o part number gravado na blindagem do módulo
#define PART_NUMBER "ESP32-S3-WROOM-1-N16R8"
#define MB (1024 * 1024)

typedef struct {
    char temperatura;   // 'N' normal ou 'H' alta
    int flash_mb;
    int psram_mb;
} memoria_t;

// Procura, do fim para o começo, o campo de memória: N16R8, H4, N4X...
static bool decodificar(const char *pn, memoria_t *m)
{
    for (const char *p = pn + strlen(pn) - 1; p > pn; p--) {
        if (p[-1] == '-' && (p[0] == 'N' || p[0] == 'H') && p[1] >= '0' && p[1] <= '9') {
            char *fim;
            m->temperatura = p[0];
            m->flash_mb = strtol(p + 1, &fim, 10);
            m->psram_mb = (*fim == 'R') ? strtol(fim + 1, NULL, 10) : 0;
            return true;
        }
    }
    return false;
}

void app_main(void)
{
    memoria_t esperado;
    if (!decodificar(PART_NUMBER, &esperado)) {
        printf("Sem campo de memória em %s\n", PART_NUMBER);
        return;
    }

    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t fisica = 0, configurada = 0;
    ESP_ERROR_CHECK(esp_flash_get_physical_size(NULL, &fisica));
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &configurada));
    int psram_mb = 0;
#if CONFIG_SPIRAM
    if (esp_psram_is_initialized()) {
        psram_mb = esp_psram_get_size() / MB;
    }
#endif

    printf("Part number: %s (flash de temperatura %s)\n", PART_NUMBER,
           esperado.temperatura == 'H' ? "alta" : "normal");
    printf("Chip: %s v%d.%d, pacote %" PRIu32 ", flash %s\n", CONFIG_IDF_TARGET,
           chip.revision / 100, chip.revision % 100, esp_efuse_get_pkg_ver(),
           (chip.features & CHIP_FEATURE_EMB_FLASH) ? "no encapsulamento" : "externa");
    printf("Flash: esperada %d MB, encontrada %d MB, projeto usa %d MB\n",
           esperado.flash_mb, (int)(fisica / MB), (int)(configurada / MB));
    printf("PSRAM: esperada %d MB, encontrada %d MB\n", esperado.psram_mb, psram_mb);

    int erros = 0;
    if ((int)(fisica / MB) != esperado.flash_mb) {
        printf("DIVERGENTE: a flash não corresponde ao part number\n");
        erros++;
    }
    if (psram_mb != esperado.psram_mb) {
        printf("DIVERGENTE: a PSRAM não corresponde ao part number\n");
        erros++;
    }
    if (configurada < fisica) {
        printf("AVISO: o projeto usa só parte da flash; ajuste o tamanho no menuconfig\n");
    }
    printf("%s\n", erros ? "Placa REPROVADA" : "Placa conforme o part number");
}
