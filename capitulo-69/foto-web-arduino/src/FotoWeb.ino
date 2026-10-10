// Foto JPEG por HTTP: abra http://<ip>/foto no navegador.
// Pinos: ESP32-CAM (AI-Thinker) no ESP32; ESP32-S3-EYE no S3.
#include <WiFi.h>
#include <WebServer.h>
#include "esp_camera.h"

const char *REDE = "minha-rede";
const char *SENHA = "minha-senha";
WebServer servidor(80);

#if CONFIG_IDF_TARGET_ESP32
const int D[8] = {5, 18, 19, 21, 36, 39, 34, 35};          // Y2 a Y9
const int XCLK = 0, SIOD = 26, SIOC = 27, VSYNC = 25, HREF = 23, PCLK = 22, PWDN = 32;
#else
const int D[8] = {11, 9, 8, 10, 12, 18, 17, 16};
const int XCLK = 15, SIOD = 4, SIOC = 5, VSYNC = 6, HREF = 7, PCLK = 13, PWDN = -1;
#endif

void foto() {
  camera_fb_t *fb = esp_camera_fb_get();                     // quadro mais recente
  if (!fb) {
    servidor.send(503, "text/plain", "sem quadro");
    return;
  }
  servidor.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);                         // devolve o buffer ao driver
}

void setup() {
  Serial.begin(115200);
  camera_config_t c = {};
  c.pin_d0 = D[0]; c.pin_d1 = D[1]; c.pin_d2 = D[2]; c.pin_d3 = D[3];
  c.pin_d4 = D[4]; c.pin_d5 = D[5]; c.pin_d6 = D[6]; c.pin_d7 = D[7];
  c.pin_xclk = XCLK; c.pin_pclk = PCLK; c.pin_vsync = VSYNC; c.pin_href = HREF;
  c.pin_sccb_sda = SIOD; c.pin_sccb_scl = SIOC;
  c.pin_pwdn = PWDN; c.pin_reset = -1;
  c.xclk_freq_hz = 20000000;                                 // MCLK do sensor
  c.ledc_timer = LEDC_TIMER_0;
  c.ledc_channel = LEDC_CHANNEL_0;
  c.pixel_format = PIXFORMAT_JPEG;                           // o sensor já comprime
  c.frame_size = FRAMESIZE_VGA;                              // 640 x 480
  c.jpeg_quality = 12;                                   // 0 a 63, menor é melhor
  c.fb_count = 2;
  c.fb_location = CAMERA_FB_IN_PSRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;
  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("câmera falhou: 0x%x\n", err);
    return;
  }
  WiFi.begin(REDE, SENHA);
  while (WiFi.status() != WL_CONNECTED) delay(250);
  servidor.on("/foto", foto);
  servidor.begin();
  Serial.printf("abra http://%s/foto\n", WiFi.localIP().toString().c_str());
}

void loop() {
  servidor.handleClient();
}
