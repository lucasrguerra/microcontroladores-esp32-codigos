#include <WiFi.h>
#include <WebServer.h>

WebServer servidor(80);

void pagina() {
  String html = "<!DOCTYPE html><meta charset='utf-8'>";
  html += "<h1>ESP32 como ponto de acesso</h1>";
  html += "<p>Estações conectadas: " + String(WiFi.softAPgetStationNum()) + "</p>";
  html += "<p>Ligado há " + String(millis() / 1000) + " s</p>";
  servidor.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-config", "senha-forte-123", 6, false, 4);   // canal 6, até 4
  Serial.printf("AP pronto: conecte em ESP32-config e abra http://%s\n",
                WiFi.softAPIP().toString().c_str());
  servidor.on("/", pagina);
  servidor.begin();
}

void loop() {
  servidor.handleClient();
}
