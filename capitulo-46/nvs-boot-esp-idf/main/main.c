#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

typedef struct {
    uint16_t versao;
    int16_t offset_temperatura;         /* décimos de grau */
    float ganho;
} calibracao_t;

static esp_err_t iniciar_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());     /* cheia ou de outra versão */
        err = nvs_flash_init();
    }
    return err;
}

void app_main(void)
{
    ESP_ERROR_CHECK(iniciar_nvs());

    nvs_handle_t h;
    ESP_ERROR_CHECK(nvs_open("app", NVS_READWRITE, &h));

    uint32_t boots = 0;                 /* fica 0 se a chave ainda não existe */
    esp_err_t err = nvs_get_u32(h, "boots", &boots);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_ERROR_CHECK(err);
    }
    boots++;
    ESP_ERROR_CHECK(nvs_set_u32(h, "boots", boots));

    calibracao_t cal;
    size_t tam = sizeof(cal);
    if (nvs_get_blob(h, "cal", &cal, &tam) != ESP_OK || tam != sizeof(cal)) {
        cal = (calibracao_t){.versao = 1, .offset_temperatura = -15, .ganho = 1.02f};
        ESP_ERROR_CHECK(nvs_set_blob(h, "cal", &cal, sizeof(cal)));
        printf("calibração padrão gravada\n");
    }
    ESP_ERROR_CHECK(nvs_commit(h));
    nvs_close(h);

    nvs_stats_t st;
    ESP_ERROR_CHECK(nvs_get_stats(NULL, &st));
    printf("boot %lu; calibração v%u, offset %d, ganho %.2f\n", (unsigned long)boots,
           cal.versao, cal.offset_temperatura, cal.ganho);
    printf("NVS: %u entradas usadas, %u livres, %u namespaces\n",
           (unsigned)st.used_entries, (unsigned)st.free_entries,
           (unsigned)st.namespace_count);

    if (boots < 3) {
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();                  /* o valor sobrevive ao reinício */
    }
}
