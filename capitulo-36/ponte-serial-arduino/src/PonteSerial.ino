const int PINO_TX = 4;              // vai ao RX do dispositivo
const int PINO_RX = 5;              // vem do TX do dispositivo
const uint32_t BAUD = 9600;         // a velocidade do dispositivo

volatile bool mensagem_completa = false;

void fim_de_mensagem() {            // roda numa tarefa do driver, não numa ISR
  mensagem_completa = true;
}

void erro_serial(hardwareSerial_error_t e) {
  Serial.printf("\n[erro na UART1: %d]\n", (int)e);
}

void setup() {
  Serial.begin(115200);
  Serial1.setRxBufferSize(1024);    // antes do begin()
  Serial1.begin(BAUD, SERIAL_8N1, PINO_RX, PINO_TX);
  Serial1.setRxTimeout(10);         // 10 caracteres de silêncio = fim da mensagem
  Serial1.onReceive(fim_de_mensagem, true);
  Serial1.onReceiveError(erro_serial);
  Serial.println("ponte pronta: digite para enviar à UART1");
}

void loop() {
  while (Serial.available()) {
    Serial1.write(Serial.read());
  }
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  if (mensagem_completa) {
    mensagem_completa = false;
    Serial.println("  <- fim de mensagem");
  }
}
