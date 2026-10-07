#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
#if SOC_WIFI_SUPPORT_5G
  WiFi.setBandMode(WIFI_BAND_MODE_AUTO);   // C5: varre 2,4 e 5 GHz
#endif
}

void loop() {
  int n = WiFi.scanNetworks();
  Serial.printf("%d rede(s) encontrada(s)\n", n);
  for (int i = 0; i < n; i++) {
    int canal = WiFi.channel(i);
    const char *banda = canal > 14 ? "5 GHz" : "2,4 GHz";
    Serial.printf("%-24s canal %3d  %-7s  %4d dBm\n",
                  WiFi.SSID(i).c_str(), canal, banda, WiFi.RSSI(i));
  }
  WiFi.scanDelete();
  delay(10000);
}
