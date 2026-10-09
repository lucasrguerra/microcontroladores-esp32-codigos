const int PINO = 4;                 // DIN da fita
const int LEDS = 8;
rmt_data_t sinal[LEDS * 24];        // 24 símbolos por LED

// 10 MHz, 1 tique = 0,1 µs. Bit 1: 0,9 µs alto + 0,3 µs baixo; bit 0: o contrário
void bit_para_simbolo(rmt_data_t *s, bool um) {
  s->level0 = 1;
  s->duration0 = um ? 9 : 3;
  s->level1 = 0;
  s->duration1 = um ? 3 : 9;
}

void pintar(int led, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;   // ordem do WS2812
  for (int i = 0; i < 24; i++) {
    bit_para_simbolo(&sinal[led * 24 + i], grb & (1UL << (23 - i)));
  }
}

void setup() {
  if (!rmtInit(PINO, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000)) {
    Serial.begin(115200);
    Serial.println("RMT indisponível neste pino ou nesta série");
  }
}

void loop() {
  static uint8_t passo = 0;
  for (int led = 0; led < LEDS; led++) {
    uint8_t fase = passo + led * 32;
    pintar(led, fase < 128 ? fase : 255 - fase, 64, 255 - fase);
  }
  rmtWrite(PINO, sinal, RMT_SYMBOLS_OF(sinal), RMT_WAIT_FOR_EVER);
  passo += 4;
  delay(20);                        // a pausa já é o reset de 50 µs
}
