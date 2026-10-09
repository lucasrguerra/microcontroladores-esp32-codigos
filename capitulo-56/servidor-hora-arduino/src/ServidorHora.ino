// Servidor web com hora certa (SNTP) e nome na rede local (mDNS).
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <time.h>

const char *SSID = "minha-rede";
const char *SENHA = "minha-senha";

WebServer servidor(80);

void paginaHora() {
  struct tm agora;
  if (!getLocalTime(&agora, 100)) {
    servidor.send(503, "application/json", "{\"erro\":\"sem hora\"}");
    return;
  }
  char txt[96];
  strftime(txt, sizeof(txt), "{\"hora\":\"%H:%M:%S\",\"data\":\"%d/%m/%Y\"}",
           &agora);
  servidor.send(200, "application/json", txt);
}

void paginaInicial() {
  servidor.send(200, "text/html",
                "<h1>ESP32</h1><p><a href=\"/hora\">/hora</a> devolve JSON.</p>");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  Serial.print("IP ");
  Serial.println(WiFi.localIP());

  // Horário de Brasília (UTC-3, sem horário de verão) e dois servidores NTP.
  configTzTime("<-03>3", "pool.ntp.org", "a.st1.ntp.br");

  if (MDNS.begin("relogio")) {               // http://relogio.local
    MDNS.addService("http", "tcp", 80);
  }
  servidor.on("/", paginaInicial);
  servidor.on("/hora", paginaHora);
  servidor.onNotFound([] { servidor.send(404, "text/plain", "nada aqui"); });
  servidor.begin();
}

void loop() {
  servidor.handleClient();
  delay(2);
}
