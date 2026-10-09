#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// ADC1 canal 0: GPIO36 no ESP32, GPIO1 no S3, GPIO0 no C3 e no C6
#define CANAL_BATERIA     ADC_CHANNEL_0
#define DIVISOR           2           // R1 = R2 = 100 kΩ
#define AMOSTRAS          16
#define BATERIA_BAIXA_MV  3400

static const char *TAG = "bateria";

static adc_cali_handle_t calibrar(void)
{
    adc_cali_handle_t cali = NULL;
    esp_err_t err = ESP_ERR_NOT_SUPPORTED;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cfg = {
        .unit_id = ADC_UNIT_1,
        .chan = CANAL_BATERIA,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_cali_create_scheme_curve_fitting(&cfg, &cali);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    err = adc_cali_create_scheme_line_fitting(&cfg, &cali);
#endif
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "sem calibração de fábrica (%s)", esp_err_to_name(err));
        return NULL;
    }
    return cali;
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc;
    adc_oneshot_unit_init_cfg_t unidade = { .unit_id = ADC_UNIT_1 };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unidade, &adc));

    adc_oneshot_chan_cfg_t canal = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, CANAL_BATERIA, &canal));
    adc_cali_handle_t cali = calibrar();

    while (1) {
        int soma = 0;
        for (int i = 0; i < AMOSTRAS; i++) {
            int bruto = 0;
            ESP_ERROR_CHECK(adc_oneshot_read(adc, CANAL_BATERIA, &bruto));
            soma += bruto;
        }
        int bruto = soma / AMOSTRAS;
        int mv = 0;
        if (cali) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, bruto, &mv));
        } else {
            mv = bruto * 3100 / 4095;   // estimativa sem calibração
        }
        int vbat = mv * DIVISOR;
        ESP_LOGI(TAG, "bruto %d, ADC %d mV, bateria %d mV", bruto, mv, vbat);
        if (vbat < BATERIA_BAIXA_MV) {
            ESP_LOGW(TAG, "bateria baixa: suspender gravações e transmissões");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
