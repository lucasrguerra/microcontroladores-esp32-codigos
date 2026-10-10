// Oito barras de cor num ST7789 240x320 por SPI, com o esp_lcd do Arduino Core.
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"

#if CONFIG_IDF_TARGET_ESP32       // no ESP32, os GPIO 6 a 11 são da flash
const int PINO_SCLK = 18, PINO_MOSI = 23, PINO_CS = 5, PINO_DC = 16, PINO_RST = 17;
#else
const int PINO_SCLK = 6, PINO_MOSI = 7, PINO_CS = 10, PINO_DC = 4, PINO_RST = 5;
#endif
const int LARGURA = 240, ALTURA = 320, FAIXA = 40;        // linhas por envio
esp_lcd_panel_handle_t painel;
uint16_t *linhas;                                          // buffer de uma faixa

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  return (c >> 8) | (c << 8);           // o ST7789 recebe o byte alto primeiro
}

void setup() {
  Serial.begin(115200);
  spi_bus_config_t bus = {};
  bus.sclk_io_num = PINO_SCLK;
  bus.mosi_io_num = PINO_MOSI;
  bus.miso_io_num = bus.quadwp_io_num = bus.quadhd_io_num = -1;
  bus.max_transfer_sz = LARGURA * FAIXA * 2;
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

  esp_lcd_panel_io_handle_t io;
  esp_lcd_panel_io_spi_config_t io_cfg = {};
  io_cfg.cs_gpio_num = PINO_CS;
  io_cfg.dc_gpio_num = PINO_DC;
  io_cfg.pclk_hz = 40 * 1000 * 1000;
  io_cfg.trans_queue_depth = 10;
  io_cfg.lcd_cmd_bits = 8;
  io_cfg.lcd_param_bits = 8;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_cfg, &io));

  esp_lcd_panel_dev_config_t dev = {};
  dev.reset_gpio_num = PINO_RST;
  dev.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  dev.bits_per_pixel = 16;
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &dev, &painel));
  esp_lcd_panel_reset(painel);
  esp_lcd_panel_init(painel);
  esp_lcd_panel_disp_on_off(painel, true);

  linhas = (uint16_t *)heap_caps_malloc(LARGURA * FAIXA * 2, MALLOC_CAP_DMA);
  const uint16_t cores[8] = { rgb565(255, 255, 255), rgb565(255, 255, 0),
                              rgb565(0, 255, 255),   rgb565(0, 255, 0),
                              rgb565(255, 0, 255),   rgb565(255, 0, 0),
                              rgb565(0, 0, 255),     rgb565(0, 0, 0) };
  for (int i = 0; i < LARGURA * FAIXA; i++) {    // as faixas são todas iguais
    linhas[i] = cores[(i % LARGURA) / (LARGURA / 8)];      // barras verticais
  }
  for (int y = 0; y < ALTURA; y += FAIXA) {                // o DMA lê o mesmo buffer
    esp_lcd_panel_draw_bitmap(painel, 0, y, LARGURA, y + FAIXA, linhas);
  }
  Serial.println("barras enviadas");
}

void loop() {}
