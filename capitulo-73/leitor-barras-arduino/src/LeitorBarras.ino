// Leitor de código de barras USB: o ESP32 é o host, o leitor é um "teclado".
// Só nas séries com USB OTG (S2, S3, P4). O leitor vai na porta OTG, com 5 V.
#include <USBHost.h>
#include <USBHostHIDKeyboard.h>

USBHostHIDKeyboard leitor;
String codigo;

// Chamado a cada relatório do teclado: até 6 teclas pressionadas ao mesmo tempo.
void aoReceber(uint8_t modificadores, const uint8_t teclas[6], void *) {
  char texto[8];
  size_t n = leitor.toAscii(texto, sizeof(texto), modificadores, teclas);
  for (size_t i = 0; i < n; i++) {
    if (texto[i] == '\n' || texto[i] == '\r') {         // o leitor termina com Enter
      if (codigo.length()) Serial.printf("código lido: %s\n", codigo.c_str());
      codigo = "";
    } else {
      codigo += texto[i];
    }
  }
  leitor.clear();
}

void setup() {
  Serial.begin(115200);
  leitor.registerWithHost();               // antes do USBHost.begin()
  leitor.setNotifyOnChangeOnly(true);      // um relatório por mudança de tecla
  leitor.setReportCallback(aoReceber);
  if (!USBHost.begin()) {
    Serial.println("falha ao iniciar o USB host");
    return;
  }
  Serial.println("pronto: conecte o leitor na porta USB OTG");
}

void loop() {
  USBHost.task();                          // processa a enumeração e os relatórios
  delay(2);
}
