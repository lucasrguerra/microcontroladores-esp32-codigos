#include <stdbool.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_idf_version.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "soc/soc_caps.h"

static const char *TAG = "desperta";

/* Botão para o GND; o pino precisa ser RTC/LP (no C2 e no C3, GPIO0 a 5). */
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define BOTAO GPIO_NUM_0            /* o botão BOOT da placa */
#else
#define BOTAO GPIO_NUM_4
#endif
#define PERIODO_US (60ULL * 1000000)

static uint32_t causas(void)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    return esp_sleep_get_wakeup_causes();
#else
    esp_sleep_wakeup_cause_t c = esp_sleep_get_wakeup_cause();
    return c == ESP_SLEEP_WAKEUP_UNDEFINED ? 0 : BIT(c);
#endif
}

static void armar_botao(void)
{
#if SOC_PM_SUPPORT_EXT0_WAKEUP                       /* ESP32, S2, S3 */
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(BOTAO, 0));
    rtc_gpio_pullup_en(BOTAO);                       /* pull-up do domínio RTC */
    rtc_gpio_pulldown_dis(BOTAO);
#elif SOC_PM_SUPPORT_EXT1_WAKEUP                     /* C5, C6, C61, H2, P4 */
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(BIT64(BOTAO),
                                                    ESP_EXT1_WAKEUP_ANY_LOW));
    rtc_gpio_pullup_en(BOTAO);
    rtc_gpio_pulldown_dis(BOTAO);
#else                                                /* C2, C3 */
    gpio_pullup_en(BOTAO);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup_on_hp_periph_powerdown(
        BIT64(BOTAO), ESP_GPIO_WAKEUP_GPIO_LOW));
#else
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT64(BOTAO),
                                                      ESP_GPIO_WAKEUP_GPIO_LOW));
#endif
#endif
}

void app_main(void)
{
    uint32_t c = causas();
    if (c & BIT(ESP_SLEEP_WAKEUP_TIMER)) {
        ESP_LOGI(TAG, "acordou pelo timer: hora de medir");
    }
    if (c & (BIT(ESP_SLEEP_WAKEUP_EXT0) | BIT(ESP_SLEEP_WAKEUP_EXT1) |
             BIT(ESP_SLEEP_WAKEUP_GPIO))) {
        ESP_LOGI(TAG, "acordou pelo botão: atende o usuário");
    }
    if (c == 0) {
        ESP_LOGI(TAG, "boot normal (energia ou reset)");
    }

    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(PERIODO_US));
    armar_botao();
    ESP_LOGI(TAG, "dormindo: timer de 60 s ou botão no GPIO%d", BOTAO);
    esp_deep_sleep_start();
}
