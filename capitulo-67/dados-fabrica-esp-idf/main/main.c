#include <stdio.h>
#include <inttypes.h>
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_app_desc.h"
#include "nvs_flash.h"
#include "nvs.h"

static bool le_texto(nvs_handle_t h, const char *chave, char *buf, size_t tam)
{
    size_t n = tam;
    return nvs_get_str(h, chave, buf, &n) == ESP_OK;
}

void app_main(void)
{
    char serie[16] = "?", lote[8] = "?", modelo[16] = "?";
    uint8_t hw = 0;
    bool ok = false;
    nvs_handle_t h;
    if (nvs_flash_init_partition("fabrica") == ESP_OK &&
        nvs_open_from_partition("fabrica", "fab", NVS_READONLY, &h) == ESP_OK) {
        ok = le_texto(h, "serie", serie, sizeof(serie)) &&
             le_texto(h, "lote", lote, sizeof(lote)) &&
             le_texto(h, "modelo", modelo, sizeof(modelo)) &&
             nvs_get_u8(h, "hw_rev", &hw) == ESP_OK;
        nvs_close(h);
    }

    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    uint32_t flash = 0;
    esp_flash_get_physical_size(NULL, &flash);
    esp_chip_info_t chip;
    esp_chip_info(&chip);

    /* Uma linha só, fácil de a estação de teste ler e arquivar. */
    printf("FAB;%s;%s;%s;hw%u;" MACSTR ";rev%u;%" PRIu32 "MB;fw%s;%s\n",
           serie, lote, modelo, hw, MAC2STR(mac), chip.revision, flash >> 20,
           esp_app_get_description()->version, ok ? "PASSOU" : "FALHOU");
}
