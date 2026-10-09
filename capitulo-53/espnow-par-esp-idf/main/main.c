#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define CANAL 6
/* MAC da outra placa (o programa imprime o seu ao iniciar; troque aqui). */
static const uint8_t PAR[6] = { 0x24, 0x0a, 0xc4, 0x00, 0x00, 0x01 };
/* 15 caracteres + o terminador = 16 bytes; iguais nas duas placas. */
static const uint8_t PMK[16] = "chave-primaria1";
static const uint8_t LMK[16] = "chave-do-par-01";

typedef struct {
    uint32_t sequencia;
    int16_t decimos_c;
} __attribute__((packed)) mensagem_t;

typedef struct {
    mensagem_t m;
    int rssi;
} recebida_t;

static const char *TAG = "espnow";
static QueueHandle_t s_fila;

/* Os dois callbacks rodam na tarefa do Wi-Fi: só copiam e avisam. */
static void ao_enviar(const esp_now_send_info_t *info, esp_now_send_status_t status)
{
    if (status != ESP_NOW_SEND_SUCCESS) {
        ESP_LOGW(TAG, "sem confirmação de " MACSTR, MAC2STR(info->des_addr));
    }
}

static void ao_receber(const esp_now_recv_info_t *info, const uint8_t *dados, int n)
{
    if (n == sizeof(mensagem_t)) {
        recebida_t r = { .rssi = info->rx_ctrl->rssi };
        memcpy(&r.m, dados, sizeof r.m);
        xQueueSend(s_fila, &r, 0);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    s_fila = xQueueCreate(8, sizeof(recebida_t));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE));

    uint8_t meu[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, meu));
    ESP_LOGI(TAG, "meu MAC: " MACSTR ", canal %d", MAC2STR(meu), CANAL);

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(ao_enviar));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(ao_receber));
    ESP_ERROR_CHECK(esp_now_set_pmk(PMK));

    esp_now_peer_info_t par = {
        .channel = CANAL,
        .ifidx = WIFI_IF_STA,
        .encrypt = true,
    };
    memcpy(par.peer_addr, PAR, 6);
    memcpy(par.lmk, LMK, 16);
    ESP_ERROR_CHECK(esp_now_add_peer(&par));

    mensagem_t m = { 0 };
    for (;;) {
        m.sequencia++;
        m.decimos_c = 250 + (int16_t)(m.sequencia % 10);     /* um valor qualquer */
        esp_now_send(PAR, (const uint8_t *)&m, sizeof m);

        recebida_t r;
        while (xQueueReceive(s_fila, &r, pdMS_TO_TICKS(2000)) == pdTRUE) {
            ESP_LOGI(TAG, "recebido #%lu: %d,%d °C, RSSI %d dBm",
                     (unsigned long)r.m.sequencia, r.m.decimos_c / 10,
                     r.m.decimos_c % 10, r.rssi);
        }
    }
}
