const int SERVO = 4;               // sinal do servo
const int LED = 5;                 // LED com resistor para o GND
const int BITS = 14;               // 14 bits cabem em todas as séries
const uint32_t MAXIMO = (1 << BITS) - 1;

// Servo comum: período de 20 ms (50 Hz), pulso de 1 ms (0°) a 2 ms (180°)
uint32_t angulo_para_duty(int graus) {
  uint32_t pulso_us = 1000 + (uint32_t)graus * 1000 / 180;
  return pulso_us * (1 << BITS) / 20000;
}

void setup() {
  Serial.begin(115200);
  ledcAttach(SERVO, 50, BITS);
  ledcAttach(LED, 5000, BITS);     // 5 kHz: sem cintilação visível
}

void loop() {
  ledcFade(LED, 0, MAXIMO, 1500);  // acende em 1,5 s, sem a CPU
  for (int g = 0; g <= 180; g += 10) {
    ledcWrite(SERVO, angulo_para_duty(g));
    delay(80);
  }
  ledcFade(LED, MAXIMO, 0, 1500);  // apaga em 1,5 s
  for (int g = 180; g >= 0; g -= 10) {
    ledcWrite(SERVO, angulo_para_duty(g));
    delay(80);
  }
  Serial.printf("ciclo completo; duty do servo em 0°: %lu de %lu\n",
                (unsigned long)angulo_para_duty(0), (unsigned long)MAXIMO);
}
