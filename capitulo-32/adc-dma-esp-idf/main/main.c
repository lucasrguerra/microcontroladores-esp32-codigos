#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

#if CONFIG_IDF_TARGET_ESP32
static const int PINOS[] = {34, 35};
#elif CONFIG_IDF_TARGET_ESP32C3
static const int PINOS[] = {3, 4};
#else
static const int PINOS[] = {4, 5};          /* S3 e C6 */
#endif
#define N_CANAIS     2
#define FREQ_HZ      20000                  /* total, somando os canais */
#define POR_QUADRO   400                    /* conversões por quadro: 20 ms */

static const char *TAG = "adc_dma";
static TaskHandle_t s_tarefa;

static bool IRAM_ATTR quadro_pronto(adc_continuous_handle_t h,
                                    const adc_continuous_evt_data_t *ev, void *arg)
{
    BaseType_t acordou = pdFALSE;
    vTaskNotifyGiveFromISR(s_tarefa, &acordou);
    return acordou == pdTRUE;
}

static adc_cali_handle_t calibrar(void)
{
    adc_cali_handle_t cali = NULL;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cfg = {
        .unit_id = ADC_UNIT_1, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cfg, &cali));
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cfg = {
        .unit_id = ADC_UNIT_1, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cfg, &cali));
#endif
    return cali;
}

void app_main(void)
{
    s_tarefa = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t adc;
    adc_continuous_handle_cfg_t hcfg = {
        .max_store_buf_size = 4 * POR_QUADRO * SOC_ADC_DIGI_RESULT_BYTES,
        .conv_frame_size = POR_QUADRO * SOC_ADC_DIGI_RESULT_BYTES,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&hcfg, &adc));

    adc_digi_pattern_config_t padrao[N_CANAIS] = {0};
    adc_channel_t canal[N_CANAIS];
    for (int i = 0; i < N_CANAIS; i++) {
        adc_unit_t unidade;
        ESP_ERROR_CHECK(adc_continuous_io_to_channel(PINOS[i], &unidade, &canal[i]));
        padrao[i].unit = unidade;
        padrao[i].channel = canal[i];
        padrao[i].atten = ADC_ATTEN_DB_12;
        padrao[i].bit_width = ADC_BITWIDTH_12;
    }
    adc_continuous_config_t cfg = {
        .pattern_num = N_CANAIS,
        .adc_pattern = padrao,
        .sample_freq_hz = FREQ_HZ,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_continuous_config(adc, &cfg));

    adc_continuous_evt_cbs_t cbs = { .on_conv_done = quadro_pronto };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc, &cbs, NULL));
    adc_cali_handle_t cali = calibrar();
    ESP_ERROR_CHECK(adc_continuous_start(adc));

    static uint8_t bruto[POR_QUADRO * SOC_ADC_DIGI_RESULT_BYTES];
    static adc_continuous_data_t amostra[POR_QUADRO];
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uint32_t bytes = 0, n = 0;
        esp_err_t r = adc_continuous_read(adc, bruto, sizeof(bruto), &bytes, 0);
        if (r == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "pool cheio: amostras perdidas");
            continue;
        }
        if (r != ESP_OK) {
            continue;
        }
        ESP_ERROR_CHECK(adc_continuous_parse_data(adc, bruto, bytes, amostra, &n));

        uint32_t soma[N_CANAIS] = {0}, cont[N_CANAIS] = {0};
        uint32_t min[N_CANAIS] = {4095, 4095}, max[N_CANAIS] = {0};
        for (uint32_t k = 0; k < n; k++) {
            for (int i = 0; i < N_CANAIS; i++) {
                if (amostra[k].valid && amostra[k].channel == canal[i]) {
                    uint32_t v = amostra[k].raw_data;
                    soma[i] += v;
                    cont[i]++;
                    if (v < min[i]) min[i] = v;
                    if (v > max[i]) max[i] = v;
                }
            }
        }
        for (int i = 0; i < N_CANAIS; i++) {
            if (cont[i] == 0) continue;
            int media = soma[i] / cont[i], mv = 0;
            if (cali) adc_cali_raw_to_voltage(cali, media, &mv);
            printf("GPIO %d: %4d mV (bruto %4d, de %4d a %4d)   ",
                   PINOS[i], mv, media, (int)min[i], (int)max[i]);
        }
        printf("\n");
    }
}
