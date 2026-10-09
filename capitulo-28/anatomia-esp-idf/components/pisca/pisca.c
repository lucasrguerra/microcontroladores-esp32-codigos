#include "driver/gpio.h"
#include "esp_timer.h"
#include "pisca.h"

static int s_gpio;
static bool s_aceso;

static void trocar(void *arg)
{
    s_aceso = !s_aceso;
    gpio_set_level(s_gpio, s_aceso);
}

esp_err_t pisca_iniciar(int gpio, uint32_t periodo_ms)
{
    s_gpio = gpio;
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        return err;
    }

    const esp_timer_create_args_t args = {
        .callback = trocar,
        .name = "pisca",
    };
    esp_timer_handle_t timer;
    err = esp_timer_create(&args, &timer);
    if (err != ESP_OK) {
        return err;
    }
    return esp_timer_start_periodic(timer, (uint64_t)periodo_ms * 1000);
}
