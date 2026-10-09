const int BOTAO = 4;               // botão entre o GPIO 4 e o GND
const uint32_t JANELA_US = 30000;  // 30 ms sem bordas = repique acabou

volatile uint32_t bordas = 0;      // todas as bordas, com repique
volatile uint32_t ultima_us = 0;   // instante da borda mais recente

void ARDUINO_ISR_ATTR na_borda() {
  bordas++;
  ultima_us = micros();
}

void setup() {
  Serial.begin(115200);
  pinMode(BOTAO, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BOTAO), na_borda, CHANGE);
}

void loop() {
  static uint32_t vistas = 0;      // bordas já tratadas
  static bool apertado = false;    // último estado estável
  static uint32_t toques = 0;

  uint32_t agora = bordas;
  if (agora != vistas && micros() - ultima_us > JANELA_US) {
    bool nivel_baixo = digitalRead(BOTAO) == LOW;
    if (nivel_baixo != apertado) {
      apertado = nivel_baixo;
      if (apertado) toques++;
      Serial.printf("%s  (toques: %lu, bordas desde o último evento: %lu)\n",
                    apertado ? "apertou" : "soltou ",
                    (unsigned long)toques, (unsigned long)(agora - vistas));
    }
    vistas = agora;
  }
  delay(1);
}
