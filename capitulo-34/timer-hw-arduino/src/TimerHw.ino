const int LED = 5;
hw_timer_t *timer = nullptr;
volatile uint32_t alarmes = 0;

void ARDUINO_ISR_ATTR no_alarme() {
  alarmes++;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  timer = timerBegin(1000000);              // contador a 1 MHz: 1 tique = 1 µs
  timerAttachInterrupt(timer, &no_alarme);
  timerAlarm(timer, 1000, true, 0);         // a cada 1000 µs, recarrega e repete
}

void loop() {
  static uint32_t vistos = 0;
  uint32_t agora = alarmes;
  if (agora - vistos >= 500) {              // 500 alarmes = 0,5 s
    vistos += 500;
    digitalWrite(LED, !digitalRead(LED));
    Serial.printf("alarmes: %lu   millis(): %lu   timer: %llu µs\n",
                  (unsigned long)agora, millis(), timerReadMicros(timer));
  }
}
