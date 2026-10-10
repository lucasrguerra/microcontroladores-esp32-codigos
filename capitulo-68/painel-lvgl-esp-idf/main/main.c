#include <sys/lock.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "lvgl.h"
#if CONFIG_PAINEL_QEMU
#include "esp_lcd_qemu_rgb.h"
#endif

#define LARGURA 240
#define ALTURA  320
#define LINHAS  40                  /* altura de cada buffer de desenho */
static const char *TAG = "painel";
static _lock_t trava_lvgl;          /* o LVGL não pode ser chamado de duas tarefas */

#if !CONFIG_PAINEL_QEMU
static bool envio_feito(esp_lcd_panel_io_handle_t io,
                        esp_lcd_panel_io_event_data_t *ev, void *ctx)
{
    lv_display_flush_ready((lv_display_t *)ctx);    /* o DMA terminou: buffer livre */
    return false;
}
#endif

static void desenha(lv_display_t *disp, const lv_area_t *a, uint8_t *pixels)
{
    esp_lcd_panel_handle_t painel = lv_display_get_user_data(disp);
#if CONFIG_PAINEL_QEMU
    esp_lcd_panel_draw_bitmap(painel, a->x1, a->y1, a->x2 + 1, a->y2 + 1, pixels);
    lv_display_flush_ready(disp);   /* o painel virtual copia na hora */
#else
    lv_draw_sw_rgb565_swap(pixels, lv_area_get_size(a));   /* byte alto primeiro */
    esp_lcd_panel_draw_bitmap(painel, a->x1, a->y1, a->x2 + 1, a->y2 + 1, pixels);
#endif
}

static esp_lcd_panel_handle_t abre_painel(lv_display_t *disp)
{
    esp_lcd_panel_handle_t painel = NULL;
#if CONFIG_PAINEL_QEMU
    esp_lcd_rgb_qemu_config_t cfg = {
        .width = LARGURA, .height = ALTURA, .bpp = RGB_QEMU_BPP_16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_qemu(&cfg, &painel));
#else
    spi_bus_config_t bus = {
        .sclk_io_num = CONFIG_PINO_SCLK, .mosi_io_num = CONFIG_PINO_MOSI,
        .miso_io_num = -1, .quadwp_io_num = -1, .quadhd_io_num = -1,
        .max_transfer_sz = LARGURA * LINHAS * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = CONFIG_PINO_CS, .dc_gpio_num = CONFIG_PINO_DC,
        .pclk_hz = 40 * 1000 * 1000, .trans_queue_depth = 10,
        .lcd_cmd_bits = 8, .lcd_param_bits = 8,
        .on_color_trans_done = envio_feito, .user_ctx = disp,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_cfg, &io));
    esp_lcd_panel_dev_config_t dev = {
        .reset_gpio_num = CONFIG_PINO_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &dev, &painel));
#endif
    ESP_ERROR_CHECK(esp_lcd_panel_reset(painel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(painel));
    esp_lcd_panel_disp_on_off(painel, true);
    return painel;
}

static void conta_tempo(void *arg)
{
    lv_tick_inc(5);                                  /* chamado a cada 5 ms */
}

static void tarefa_lvgl(void *arg)
{
    while (true) {
        _lock_acquire(&trava_lvgl);
        uint32_t espera = lv_timer_handler();        /* redesenha o que mudou */
        _lock_release(&trava_lvgl);
        vTaskDelay(pdMS_TO_TICKS(espera < 10 ? 10 : (espera > 500 ? 500 : espera)));
    }
}

static void monta_tela(void)
{
    lv_obj_t *tela = lv_screen_active();
    lv_obj_set_style_bg_color(tela, lv_color_white(), 0);

    lv_obj_t *titulo = lv_label_create(tela);
    lv_label_set_text(titulo, "Estufa 3");
    lv_obj_set_style_text_font(titulo, &lv_font_montserrat_28, 0);
    lv_obj_align(titulo, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *arco = lv_arc_create(tela);           /* temperatura: 0 a 50 °C */
    lv_obj_set_size(arco, 180, 180);
    lv_arc_set_range(arco, 0, 50);
    lv_arc_set_value(arco, 27);
    lv_obj_remove_flag(arco, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(arco, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *temp = lv_label_create(tela);
    lv_label_set_text(temp, "27 °C");
    lv_obj_set_style_text_font(temp, &lv_font_montserrat_28, 0);
    lv_obj_align_to(temp, arco, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *barra = lv_bar_create(tela);          /* umidade: 0 a 100 % */
    lv_obj_set_size(barra, 180, 18);
    lv_bar_set_value(barra, 64, LV_ANIM_OFF);
    lv_obj_align(barra, LV_ALIGN_BOTTOM_MID, 0, -40);

    lv_obj_t *umid = lv_label_create(tela);
    lv_label_set_text(umid, "Umidade 64 %");
    lv_obj_align_to(umid, barra, LV_ALIGN_OUT_TOP_MID, 0, -6);
}

void app_main(void)
{
    lv_init();
    lv_display_t *disp = lv_display_create(LARGURA, ALTURA);
    esp_lcd_panel_handle_t painel = abre_painel(disp);
    lv_display_set_user_data(disp, painel);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

    size_t tam = LARGURA * LINHAS * 2;              /* dois buffers de 40 linhas */
    void *b1 = heap_caps_malloc(tam, MALLOC_CAP_DMA);
    void *b2 = heap_caps_malloc(tam, MALLOC_CAP_DMA);
    lv_display_set_buffers(disp, b1, b2, tam, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, desenha);

    const esp_timer_create_args_t tick = { .callback = conta_tempo, .name = "lv" };
    esp_timer_handle_t t;
    ESP_ERROR_CHECK(esp_timer_create(&tick, &t));
    ESP_ERROR_CHECK(esp_timer_start_periodic(t, 5000));
    xTaskCreate(tarefa_lvgl, "lvgl", 6 * 1024, NULL, 2, NULL);

    _lock_acquire(&trava_lvgl);
    monta_tela();
    _lock_release(&trava_lvgl);
    ESP_LOGI(TAG, "tela montada");
}
