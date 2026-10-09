#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define PINO_SDA   4
#define PINO_SCL   5
#define REG_ID     0xD0                 /* identificação nos sensores Bosch */

static const char *TAG = "i2c";

static const char *nome_bosch(uint8_t id)
{
    switch (id) {
    case 0x60: return "BME280";
    case 0x58: return "BMP280";
    default:   return "desconhecido";
    }
}

void app_main(void)
{
    i2c_master_bus_handle_t barramento;
    i2c_master_bus_config_t bcfg = {
        .i2c_port = -1,                 /* qualquer porta livre */
        .sda_io_num = PINO_SDA,
        .scl_io_num = PINO_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,   /* use pull-ups externos */
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bcfg, &barramento));

    for (uint16_t end = 0x08; end <= 0x77; end++) {
        esp_err_t r = i2c_master_probe(barramento, end, 50);
        if (r == ESP_OK) {
            ESP_LOGI(TAG, "dispositivo em 0x%02X", end);
        } else if (r == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "timeout em 0x%02X: barramento preso?", end);
            break;
        }
    }

    for (uint16_t end = 0x76; end <= 0x77; end++) {
        if (i2c_master_probe(barramento, end, 50) != ESP_OK) {
            continue;
        }
        i2c_master_dev_handle_t sensor;
        i2c_device_config_t dcfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = end,
            .scl_speed_hz = 400000,
        };
        ESP_ERROR_CHECK(i2c_master_bus_add_device(barramento, &dcfg, &sensor));
        uint8_t reg = REG_ID, id = 0;
        esp_err_t r = i2c_master_transmit_receive(sensor, &reg, 1, &id, 1, 100);
        if (r == ESP_OK) {
            ESP_LOGI(TAG, "0x%02X: ID 0x%02X (%s)", end, id, nome_bosch(id));
        } else {
            ESP_LOGW(TAG, "0x%02X não leu o ID: %s", end, esp_err_to_name(r));
        }
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(sensor));
    }
}
