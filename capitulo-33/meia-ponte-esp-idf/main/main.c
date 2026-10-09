#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PINO_A      4               /* lado alto da meia ponte */
#define PINO_B      5               /* lado baixo */
#define PINO_FALHA  18              /* sobe para 1 em sobrecorrente */
#define RESOLUCAO   10000000        /* 10 MHz: 0,1 µs por tique */
#define PERIODO     500             /* 500 tiques = 20 kHz */
#define MORTO       5               /* 5 tiques = 0,5 µs */

static const char *TAG = "ponte";
static TaskHandle_t s_tarefa;

/* Atalhos para as regras de evento: o timer sempre conta para cima aqui. */
static void no_zero(mcpwm_gen_handle_t g, mcpwm_generator_action_t acao)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(g,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                     MCPWM_TIMER_EVENT_EMPTY, acao)));
}

static void no_comparador(mcpwm_gen_handle_t g, mcpwm_cmpr_handle_t c,
                          mcpwm_generator_action_t acao)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(g,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, c, acao)));
}

static void desliga_no_freio(mcpwm_gen_handle_t g)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_brake_event(g,
        MCPWM_GEN_BRAKE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                     MCPWM_OPER_BRAKE_MODE_OST,
                                     MCPWM_GEN_ACTION_LOW)));
}

static bool IRAM_ATTR falha_saiu(mcpwm_fault_handle_t f,
                                 const mcpwm_fault_event_data_t *ev, void *arg)
{
    BaseType_t acordou = pdFALSE;
    vTaskNotifyGiveFromISR(s_tarefa, &acordou);
    return acordou == pdTRUE;
}

void app_main(void)
{
    s_tarefa = xTaskGetCurrentTaskHandle();

    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t tcfg = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = RESOLUCAO,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = PERIODO,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&tcfg, &timer));

    mcpwm_oper_handle_t oper;
    mcpwm_operator_config_t ocfg = { .group_id = 0 };
    ESP_ERROR_CHECK(mcpwm_new_operator(&ocfg, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    mcpwm_cmpr_handle_t cmp;
    mcpwm_comparator_config_t ccfg = { .flags.update_cmp_on_tez = true };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &ccfg, &cmp));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmp, PERIODO * 40 / 100));

    mcpwm_gen_handle_t ga, gb;
    mcpwm_generator_config_t gcfg = { .gen_gpio_num = PINO_A };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gcfg, &ga));
    gcfg.gen_gpio_num = PINO_B;
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gcfg, &gb));

    /* A: sobe no início do período, desce no comparador. B: o contrário. */
    no_zero(ga, MCPWM_GEN_ACTION_HIGH);
    no_comparador(ga, cmp, MCPWM_GEN_ACTION_LOW);
    no_zero(gb, MCPWM_GEN_ACTION_LOW);
    no_comparador(gb, cmp, MCPWM_GEN_ACTION_HIGH);

    /* Atrasa só as subidas: quem liga espera o outro desligar. */
    mcpwm_dead_time_config_t morto = { .posedge_delay_ticks = MORTO };
    ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(ga, ga, &morto));
    ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(gb, gb, &morto));

    mcpwm_fault_handle_t falha;
    mcpwm_gpio_fault_config_t fcfg = {
        .group_id = 0,
        .gpio_num = PINO_FALHA,
        .flags.active_level = 1,
    };
    ESP_ERROR_CHECK(mcpwm_new_gpio_fault(&fcfg, &falha));
    ESP_ERROR_CHECK(gpio_pulldown_en(PINO_FALHA));        /* sem sinal = sem falha */
    mcpwm_brake_config_t freio = {
        .fault = falha,
        .brake_mode = MCPWM_OPER_BRAKE_MODE_OST,
    };
    ESP_ERROR_CHECK(mcpwm_operator_set_brake_on_fault(oper, &freio));
    desliga_no_freio(ga);
    desliga_no_freio(gb);
    mcpwm_fault_event_callbacks_t fcbs = { .on_fault_exit = falha_saiu };
    ESP_ERROR_CHECK(mcpwm_fault_register_event_callbacks(falha, &fcbs, NULL));

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
    ESP_LOGI(TAG, "20 kHz, 40 %%, tempo morto de 0,5 µs");

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        ESP_LOGW(TAG, "a falha passou; religando as saídas");
        ESP_ERROR_CHECK(mcpwm_operator_recover_from_fault(oper, falha));
    }
}
