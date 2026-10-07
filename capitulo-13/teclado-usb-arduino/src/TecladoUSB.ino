#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard teclado;
const int BOTAO = 0;          // botão BOOT das DevKits

void setup() {
  pinMode(BOTAO, INPUT_PULLUP);
  teclado.begin();
  USB.begin();                // enumera no computador como teclado
}

void loop() {
  if (digitalRead(BOTAO) == LOW) {
    teclado.println("Olá do ESP32-S2!");
    delay(500);               // evita repetir enquanto o botão está apertado
  }
}
