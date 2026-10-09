#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "soc/soc_caps.h"
#if SOC_GPIO_SUPPORT_PIN_GLITCH_FILTER
#include "driver/gpio_filter.h"
#endif

#define BOTAO      GPIO_NUM_4      /* botão entre o GPIO 4 e o GND */
#define LED        GPIO_NUM_5      /* LED com resistor para o GND */
#define JANELA_MS  30

static const char *TAG = "botao";
static TaskHandle_t s_tarefa;

static void IRAM_ATTR na_borda(void *arg)
{
    BaseType_t acordou = pdFALSE;
    gpio_intr_disable(BOTAO);                  /* ignora o resto do repique */
    vTaskNotifyGiveFromISR(s_tarefa, &acordou);
    portYIELD_FROM_ISR(acordou);
}

static void tarefa_botao(void *arg)
{
    int estavel = 1;                           /* solto: pull-up em 1 */
    int led = 0;
    unsigned toques = 0;
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(JANELA_MS));
        int nivel = gpio_get_level(BOTAO);
        if (nivel != estavel) {
            estavel = nivel;
            if (nivel == 0) {
                led = !led;
                gpio_set_level(LED, led);
                ESP_LOGI(TAG, "toque %u, LED %s", ++toques, led ? "aceso" : "apagado");
            }
        }
        gpio_intr_enable(BOTAO);
    }
}

void app_main(void)
{
    gpio_config_t led = {
        .pin_bit_mask = 1ULL << LED,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&led));

    gpio_config_t botao = {
        .pin_bit_mask = 1ULL << BOTAO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&botao));

#if SOC_GPIO_SUPPORT_PIN_GLITCH_FILTER
    gpio_glitch_filter_handle_t filtro;
    gpio_pin_glitch_filter_config_t cfg_filtro = {
        .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
        .gpio_num = BOTAO,
    };
    ESP_ERROR_CHECK(gpio_new_pin_glitch_filter(&cfg_filtro, &filtro));
    ESP_ERROR_CHECK(gpio_glitch_filter_enable(filtro));
#endif

    xTaskCreate(tarefa_botao, "botao", 3072, NULL, 5, &s_tarefa);
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BOTAO, na_borda, NULL));
    ESP_LOGI(TAG, "aperte o botão no GPIO %d", BOTAO);
}
