#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"

static const char *TAG = "usj";

void app_main(void)
{
    usb_serial_jtag_driver_config_t cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));

    uint8_t buf[64];
    bool estava = false;
    while (1) {
        bool conectado = usb_serial_jtag_is_connected();
        if (conectado != estava) {
            ESP_LOGI(TAG, "computador %s", conectado ? "conectado" : "desconectado");
            estava = conectado;
        }
        int n = usb_serial_jtag_read_bytes(buf, sizeof(buf), pdMS_TO_TICKS(100));
        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }
        if (n > 0) {
            usb_serial_jtag_write_bytes(buf, n, pdMS_TO_TICKS(100));
        }
    }
}
