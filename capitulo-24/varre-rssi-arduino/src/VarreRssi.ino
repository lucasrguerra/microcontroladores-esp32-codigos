#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
}

void loop() {
  int n = WiFi.scanNetworks();
  Serial.printf("%d redes\n", n);
  for (int i = 0; i < n; i++) {
    Serial.printf("%4d dBm  canal %2d  %s\n", WiFi.RSSI(i), WiFi.channel(i), WiFi.SSID(i).c_str());
  }
  WiFi.scanDelete();
  delay(5000);
}
