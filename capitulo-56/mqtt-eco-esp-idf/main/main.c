#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "protocol_examples_common.h"

static const char *TAG = "mqtt";
static char topico[48];
static volatile bool conectado;

static void ao_evento(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    esp_mqtt_event_handle_t ev = dados;
    switch ((esp_mqtt_event_id_t)id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "conectado ao broker");
        esp_mqtt_client_subscribe(ev->client, topico, 1);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "inscrito em %s", topico);
        conectado = true;
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "recebido em %.*s: %.*s", ev->topic_len, ev->topic,
                 ev->data_len, ev->data);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "desconectado; o cliente tenta de novo sozinho");
        conectado = false;
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "erro de transporte");
        break;
    default:
        break;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BASE);
    snprintf(topico, sizeof(topico), "livro-esp32/%02x%02x%02x/eco",
             mac[3], mac[4], mac[5]);

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org:1883",
        .session.keepalive = 30,
        .session.last_will = {
            .topic = topico, .msg = "offline", .qos = 1, .retain = 1,
        },
    };
    esp_mqtt_client_handle_t cli = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(cli, ESP_EVENT_ANY_ID, ao_evento, NULL);
    esp_mqtt_client_start(cli);

    for (int n = 1; n <= 3; n++) {
        while (!conectado) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        char msg[32];
        int len = snprintf(msg, sizeof(msg), "leitura %d", n);
        int id = esp_mqtt_client_publish(cli, topico, msg, len, 1, 0);
        ESP_LOGI(TAG, "publicado \"%s\" (id %d)", msg, id);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
