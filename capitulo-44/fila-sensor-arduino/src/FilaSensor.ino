typedef struct {
  uint32_t instante;                // millis() da leitura
  int valor;                        // leitura bruta do ADC
} leitura_t;

QueueHandle_t fila;
volatile uint32_t perdidas = 0;

void sensor(void *arg) {
  TickType_t ultimo = xTaskGetTickCount();
  for (;;) {
    leitura_t l = { millis(), analogRead(A0) };
    if (xQueueSend(fila, &l, 0) != pdTRUE) {
      perdidas++;                   // fila cheia: o consumidor atrasou
    }
    vTaskDelayUntil(&ultimo, pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  fila = xQueueCreate(20, sizeof(leitura_t));        // 2 s de folga
  xTaskCreate(sensor, "sensor", 3072, NULL, 5, NULL);
}

void loop() {
  leitura_t l;
  if (xQueueReceive(fila, &l, pdMS_TO_TICKS(500)) == pdTRUE) {
    Serial.printf("%lu ms: %d (na fila: %u, perdidas: %lu)\n",
                  (unsigned long)l.instante, l.valor,
                  (unsigned)uxQueueMessagesWaiting(fila),
                  (unsigned long)perdidas);
  } else {
    Serial.println("nenhuma leitura em 500 ms: o sensor parou?");
  }
}
