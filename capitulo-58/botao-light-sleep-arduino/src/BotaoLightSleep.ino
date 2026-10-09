// Light sleep acordado por botão em qualquer pino (todas as séries) ou pelo timer.
#include "esp_sleep.h"
#include "driver/gpio.h"

const gpio_num_t BOTAO = (gpio_num_t)BOOT_PIN;  // o botão BOOT da placa
int toques = 0;                                 // fica na RAM: o light sleep retém

void setup() {
  Serial.begin(115200);
  pinMode(BOTAO, INPUT_PULLUP);
  // Nível baixo (botão apertado) acorda o chip; vale para qualquer GPIO.
  gpio_wakeup_enable(BOTAO, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_sleep_enable_timer_wakeup(30 * 1000000ULL);  // e a cada 30 s, de qualquer jeito
}

void loop() {
  Serial.flush();
  esp_light_sleep_start();

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
    toques++;
    Serial.printf("botão: %d toque(s)\n", toques);
    while (digitalRead(BOTAO) == LOW) {         // espera soltar, senão acorda de novo
      delay(10);
    }
  } else {
    Serial.printf("timer: ainda %d toque(s)\n", toques);
  }
}
