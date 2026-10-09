const int LED = 5;
TaskHandle_t h_pisca = nullptr;
volatile uint32_t leituras = 0;

void pisca(void *arg) {
  pinMode(LED, OUTPUT);
  for (;;) {
    digitalWrite(LED, !digitalRead(LED));
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

void sensor(void *arg) {
  TickType_t ultimo = xTaskGetTickCount();
  for (;;) {
    analogRead(A0);                                  // lê no ritmo certo
    leituras++;
    vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(10));     // 100 vezes por segundo
  }
}

void setup() {
  Serial.begin(115200);
  xTaskCreate(pisca, "pisca", 2048, NULL, 1, &h_pisca);
  xTaskCreate(sensor, "sensor", 3072, NULL, 5, NULL);
}

void loop() {
  static char lista[768];
  delay(5000);                                        // o LED e o sensor seguem
  Serial.printf("loop no núcleo %d; %lu leituras; pilha livre: loop %u, pisca %u\n",
                xPortGetCoreID(), (unsigned long)leituras,
                (unsigned)uxTaskGetStackHighWaterMark(NULL),
                (unsigned)uxTaskGetStackHighWaterMark(h_pisca));
  vTaskList(lista);
  Serial.print(lista);
}
