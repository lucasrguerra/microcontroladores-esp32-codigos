// Economia no Arduino: CPU a 80 MHz e Wi-Fi em modem sleep máximo.
#include <WiFi.h>

const char *SSID = "minha-rede";
const char *SENHA = "minha-senha";

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(80);                 // de 160 ou 240 MHz para 80 MHz
  Serial.printf("CPU a %lu MHz\n", getCpuFrequencyMhz());

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  // MIN_MODEM (padrão) acorda a cada DTIM; MAX_MODEM usa o listen interval.
  WiFi.setSleep(WIFI_PS_MAX_MODEM);
  Serial.print("conectado, IP ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.printf("RSSI %d dBm\n", WiFi.RSSI());
  delay(10000);                           // a CPU fica ociosa; o rádio cochila
}
