#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/soc_caps.h"

static const char *TAG = "no";

#if CONFIG_IDF_TARGET_ESP32
#define PINO_BATERIA 34
#elif CONFIG_IDF_TARGET_ESP32S3
#define PINO_BATERIA 4
#else
#define PINO_BATERIA 2
#endif
#define LIGA_DIVISOR GPIO_NUM_5     /* aciona o MOSFET que liga o divisor 1:2 */
#define PERIODO_US   (300ULL * 1000000)
#define CANAL        6

static RTC_DATA_ATTR uint32_t envios;
static RTC_DATA_ATTR uint32_t acordado_ms_total;   /* para a média de tempo ativo */
static SemaphoreHandle_t enviado;

typedef struct __attribute__((packed)) {
    uint32_t seq;
    uint16_t bateria_mv;
    uint16_t acordado_ms;            /* do despertar anterior */
} leitura_t;

static int ler_bateria_mv(void)
{
    adc_oneshot_unit_handle_t adc;
    adc_unit_t unid;
    adc_channel_t canal;
    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(PINO_BATERIA, &unid, &canal));
    adc_oneshot_unit_init_cfg_t ucfg = { .unit_id = unid };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&ucfg, &adc));
    adc_oneshot_chan_cfg_t ccfg = { .atten = ADC_ATTEN_DB_12,
                                    .bitwidth = ADC_BITWIDTH_DEFAULT };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, canal, &ccfg));

    adc_cali_handle_t cali = NULL;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cc = { .unit_id = unid, .chan = canal,
                                           .atten = ADC_ATTEN_DB_12 };
    adc_cali_create_scheme_curve_fitting(&cc, &cali);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t lc = { .unit_id = unid, .atten = ADC_ATTEN_DB_12 };
    adc_cali_create_scheme_line_fitting(&lc, &cali);
#endif

    gpio_hold_dis(LIGA_DIVISOR);     /* solta a trava do sono anterior */
    gpio_set_direction(LIGA_DIVISOR, GPIO_MODE_OUTPUT);
    gpio_set_level(LIGA_DIVISOR, 1);
    vTaskDelay(pdMS_TO_TICKS(2));    /* o capacitor do divisor carrega */
    int soma = 0, bruto, mv;
    for (int i = 0; i < 8; i++) {
        adc_oneshot_read(adc, canal, &bruto);
        soma += bruto;
    }
    gpio_set_level(LIGA_DIVISOR, 0); /* divisor desligado: zero de vazamento */
    if (cali == NULL || adc_cali_raw_to_voltage(cali, soma / 8, &mv) != ESP_OK) {
        mv = 0;
    }
    adc_oneshot_del_unit(adc);
    return mv * 2;                   /* desfaz o divisor 1:2 */
}

static void ao_enviar(const esp_now_send_info_t *info, esp_now_send_status_t st)
{
    xSemaphoreGive(enviado);
}

static void enviar(const leitura_t *l)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_config_t ini = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&ini));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CANAL, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(ao_enviar));
    esp_now_peer_info_t par = { .channel = CANAL, .ifidx = WIFI_IF_STA };
    memset(par.peer_addr, 0xFF, 6);  /* difusão: sem ACK de enlace */
    ESP_ERROR_CHECK(esp_now_add_peer(&par));
    esp_now_send(par.peer_addr, (const uint8_t *)l, sizeof(*l));
    xSemaphoreTake(enviado, pdMS_TO_TICKS(50));
    esp_wifi_stop();                 /* rádio desligado antes de dormir */
}

void app_main(void)
{
    static RTC_DATA_ATTR uint16_t acordado_antes;
    enviado = xSemaphoreCreateBinary();
    leitura_t l = { .seq = ++envios, .bateria_mv = ler_bateria_mv(),
                    .acordado_ms = acordado_antes };
    enviar(&l);

    uint32_t ms = esp_timer_get_time() / 1000;    /* desde o boot */
    acordado_antes = ms;
    acordado_ms_total += ms;
    ESP_LOGI(TAG, "#%lu: %u mV, %lu ms acordado (média %lu ms)",
             (unsigned long)l.seq, l.bateria_mv, (unsigned long)ms,
             (unsigned long)(acordado_ms_total / envios));
    gpio_hold_en(LIGA_DIVISOR);      /* o MOSFET fica desligado durante o sono */
#if !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
    gpio_deep_sleep_hold_en();       /* ESP32, S2, S3: hold digital no deep sleep */
#endif
    esp_deep_sleep(PERIODO_US);
}
