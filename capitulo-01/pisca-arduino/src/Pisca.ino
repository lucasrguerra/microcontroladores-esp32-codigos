const int PINO_LED = 4;   // GPIO onde o LED está ligado

void setup() {
  pinMode(PINO_LED, OUTPUT);       // configura o pino como saída
  Serial.begin(115200);
  Serial.println("Pisca iniciado");
}

void loop() {
  digitalWrite(PINO_LED, HIGH);    // 3,3 V no pino: LED aceso
  delay(500);                      // espera 500 ms
  digitalWrite(PINO_LED, LOW);     // 0 V no pino: LED apagado
  delay(500);
}
