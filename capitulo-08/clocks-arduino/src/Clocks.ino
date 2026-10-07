void mostrar(const char *quando) {
  Serial.printf("[%s] CPU %lu MHz | XTAL %lu MHz | APB %lu Hz\n", quando,
                (unsigned long)getCpuFrequencyMhz(), (unsigned long)getXtalFrequencyMhz(),
                (unsigned long)getApbFrequency());
}

void setup() {
  Serial.begin(115200);
  delay(500);
  mostrar("início");
  if (setCpuFrequencyMhz(80)) {          // só aceita frequências válidas da série
    mostrar("depois");
  }
}

void loop() {}
