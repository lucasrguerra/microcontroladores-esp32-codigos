#include "esp_twai.h"
#include "esp_twai_onchip.h"

const int PINO_TX = 4;              // ligue o GPIO 4 ao GPIO 5 (sem transceptor)
const int PINO_RX = 5;

twai_node_handle_t no;
volatile uint32_t recebidos = 0;
volatile uint32_t ultimo_id = 0;

bool IRAM_ATTR chegou(twai_node_handle_t h, const twai_rx_done_event_data_t *ev,
                      void *arg) {
  uint8_t dados[8];
  twai_frame_t q = {};
  q.buffer = dados;
  q.buffer_len = sizeof(dados);
  if (twai_node_receive_from_isr(h, &q) == ESP_OK) {
    ultimo_id = q.header.id;
    recebidos++;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  twai_onchip_node_config_t cfg = {};
  cfg.io_cfg.tx = (gpio_num_t)PINO_TX;
  cfg.io_cfg.rx = (gpio_num_t)PINO_RX;
  cfg.io_cfg.quanta_clk_out = GPIO_NUM_NC;
  cfg.io_cfg.bus_off_indicator = GPIO_NUM_NC;
  cfg.bit_timing.bitrate = 500000;
  cfg.tx_queue_depth = 4;
  cfg.flags.enable_self_test = 1;   // não espera ACK de outro nó
  cfg.flags.enable_loopback = 1;    // recebe o que ele mesmo transmite
  ESP_ERROR_CHECK(twai_new_node_onchip(&cfg, &no));
  twai_event_callbacks_t cbs = {};
  cbs.on_rx_done = chegou;
  ESP_ERROR_CHECK(twai_node_register_event_callbacks(no, &cbs, NULL));
  ESP_ERROR_CHECK(twai_node_enable(no));
}

void loop() {
  static uint8_t contador = 0;
  static uint8_t carga[2];
  carga[0] = contador;
  carga[1] = 0xA5;
  twai_frame_t q = {};
  q.header.id = 0x100 + (contador & 0x0F);
  q.header.dlc = 2;
  q.buffer = carga;
  q.buffer_len = sizeof(carga);
  ESP_ERROR_CHECK(twai_node_transmit(no, &q, 100));
  contador++;
  delay(500);
  Serial.printf("enviados: %u, recebidos: %lu, último ID: 0x%03lX\n", contador,
                (unsigned long)recebidos, (unsigned long)ultimo_id);
}
