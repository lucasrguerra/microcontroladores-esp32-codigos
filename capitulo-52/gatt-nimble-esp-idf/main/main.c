#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "gatt";

/* 6e1a0001-4b7c-4a8e-9f00-3c2a5d6e7f80 e vizinhos, em ordem inversa de bytes */
static const ble_uuid128_t SERVICO = BLE_UUID128_INIT(0x80, 0x7f, 0x6e, 0x5d, 0x2a,
    0x3c, 0x00, 0x9f, 0x8e, 0x4a, 0x7c, 0x4b, 0x01, 0x00, 0x1a, 0x6e);
static const ble_uuid128_t CONTADOR = BLE_UUID128_INIT(0x80, 0x7f, 0x6e, 0x5d, 0x2a,
    0x3c, 0x00, 0x9f, 0x8e, 0x4a, 0x7c, 0x4b, 0x02, 0x00, 0x1a, 0x6e);

static uint16_t s_conexao = BLE_HS_CONN_HANDLE_NONE;
static uint16_t s_alca_contador;        /* handle do valor, para notificar */
static bool s_inscrito;
static uint32_t s_contador;

static int acesso(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt,
                  void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        return os_mbuf_append(ctxt->om, &s_contador, sizeof s_contador) == 0
               ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint32_t v;
        if (OS_MBUF_PKTLEN(ctxt->om) != sizeof v) {
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        os_mbuf_copydata(ctxt->om, 0, sizeof v, &v);
        s_contador = v;                 /* o cliente pode zerar ou ajustar */
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def s_servicos[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &SERVICO.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &CONTADOR.u,
                .access_cb = acesso,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE |
                         BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &s_alca_contador,
            },
            { 0 },
        },
    },
    { 0 },
};

static void anunciar(void);

static int ao_evento_gap(struct ble_gap_event *ev, void *arg)
{
    switch (ev->type) {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "conexão %s", ev->connect.status == 0 ? "aberta" : "falhou");
        if (ev->connect.status == 0) {
            s_conexao = ev->connect.conn_handle;
        } else {
            anunciar();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "desconectado, motivo 0x%x", ev->disconnect.reason);
        s_conexao = BLE_HS_CONN_HANDLE_NONE;
        s_inscrito = false;
        anunciar();
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        if (ev->subscribe.attr_handle == s_alca_contador) {
            s_inscrito = ev->subscribe.cur_notify;
            ESP_LOGI(TAG, "notificações %s", s_inscrito ? "ligadas" : "desligadas");
        }
        break;
    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU negociado: %u bytes", ev->mtu.value);
        break;
    default:
        break;
    }
    return 0;
}

static void anunciar(void)
{
    struct ble_hs_adv_fields campos = { 0 };
    const char *nome = ble_svc_gap_device_name();
    campos.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    campos.name = (const uint8_t *)nome;
    campos.name_len = strlen(nome);
    campos.name_is_complete = 1;
    ble_gap_adv_set_fields(&campos);

    struct ble_gap_adv_params p = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,     /* aceita conexões */
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        .itvl_min = BLE_GAP_ADV_ITVL_MS(100),
        .itvl_max = BLE_GAP_ADV_ITVL_MS(150),
    };
    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &p,
                      ao_evento_gap, NULL);
}

static void tarefa_nimble(void *arg)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nimble_port_init());
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_gap_device_name_set("ESP32-contador");
    ESP_ERROR_CHECK(ble_gatts_count_cfg(s_servicos));
    ESP_ERROR_CHECK(ble_gatts_add_svcs(s_servicos));
    ble_hs_cfg.sync_cb = anunciar;
    nimble_port_freertos_init(tarefa_nimble);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        s_contador++;
        if (s_conexao != BLE_HS_CONN_HANDLE_NONE && s_inscrito) {
            ble_gatts_chr_updated(s_alca_contador);     /* notifica o novo valor */
        }
    }
}
