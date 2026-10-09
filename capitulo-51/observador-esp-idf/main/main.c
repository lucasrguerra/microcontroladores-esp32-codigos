#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"

static const char *TAG = "observador";

static int ao_evento_gap(struct ble_gap_event *ev, void *arg)
{
    if (ev->type == BLE_GAP_EVENT_DISC) {
        struct ble_hs_adv_fields campos;
        char nome[32] = "(sem nome)";
        int rc = ble_hs_adv_parse_fields(&campos, ev->disc.data, ev->disc.length_data);
        if (rc == 0 && campos.name_len > 0) {
            int n = campos.name_len > 31 ? 31 : campos.name_len;
            memcpy(nome, campos.name, n);
            nome[n] = '\0';
        }
        const uint8_t *a = ev->disc.addr.val;
        ESP_LOGI(TAG, "%02x:%02x:%02x:%02x:%02x:%02x  RSSI %4d dBm  %s",
                 a[5], a[4], a[3], a[2], a[1], a[0], ev->disc.rssi, nome);
    } else if (ev->type == BLE_GAP_EVENT_DISC_COMPLETE) {
        ESP_LOGI(TAG, "varredura encerrada");
    }
    return 0;
}

static void ao_sincronizar(void)
{
    uint8_t tipo_endereco;
    ble_hs_id_infer_auto(0, &tipo_endereco);
    struct ble_gap_disc_params p = {
        .itvl = BLE_GAP_SCAN_ITVL_MS(100),
        .window = BLE_GAP_SCAN_WIN_MS(50),      /* escuta metade do tempo */
        .filter_duplicates = 1,                 /* cada anunciante uma vez */
        .passive = 1,                           /* só escuta, não pede mais dados */
    };
    int rc = ble_gap_disc(tipo_endereco, 10000, &p, ao_evento_gap, NULL);
    ESP_LOGI(TAG, "varrendo por 10 s (rc=%d)", rc);
}

static void tarefa_nimble(void *arg)
{
    nimble_port_run();                  /* só retorna quando a pilha para */
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nimble_port_init());
    ble_hs_cfg.sync_cb = ao_sincronizar;
    nimble_port_freertos_init(tarefa_nimble);
}
