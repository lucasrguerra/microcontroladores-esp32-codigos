#include <ctype.h>
#include "esp_log.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_cdc_acm.h"

static const char *TAG = "usb_eco";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE];

static void ao_receber(int itf, cdcacm_event_t *evento)
{
    size_t n = 0;
    if (tinyusb_cdcacm_read(itf, buf, sizeof(buf), &n) != ESP_OK || n == 0) {
        return;
    }
    for (size_t i = 0; i < n; i++) {
        buf[i] = (uint8_t)toupper(buf[i]);
    }
    tinyusb_cdcacm_write_queue(itf, buf, n);
    tinyusb_cdcacm_write_flush(itf, 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando a TinyUSB");

    const tinyusb_config_t usb = TINYUSB_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(tinyusb_driver_install(&usb));

    tinyusb_config_cdcacm_t acm = {
        .cdc_port = TINYUSB_CDC_ACM_0,
        .callback_rx = ao_receber,
    };
    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm));
    ESP_LOGI(TAG, "Porta serial USB pronta; abra /dev/ttyACM0 no computador");
}
