const int SAIDA = 4;      // ligue um fio entre o GPIO 4...
const int ENTRADA = 5;    // ...e o GPIO 5

void setup() {
  pinMode(SAIDA, OUTPUT);
  pinMode(ENTRADA, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  for (int nivel = 0; nivel <= 1; nivel++) {
    digitalWrite(SAIDA, nivel);
    delayMicroseconds(10);
    Serial.printf("escrevi %d, li %d\n", nivel, digitalRead(ENTRADA));
  }
  delay(1000);
}
