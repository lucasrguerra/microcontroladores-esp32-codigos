# Microcontroladores ESP32: códigos do livro

Códigos-fonte dos exemplos do livro **Microcontroladores ESP32: Conectividade,
Eficiência e Desempenho**, de Lucas Rayan Guerra (Ciência Embarcada).

Cada exemplo é um projeto [PlatformIO](https://platformio.org) completo, pronto
para compilar e gravar, em ESP-IDF ou em Arduino, com um ambiente para cada
série de ESP32 em que ele foi testado.

[![Compilar exemplos](../../actions/workflows/compilar.yml/badge.svg)](../../actions/workflows/compilar.yml)

## Como usar

1. Instale o [PlatformIO](https://platformio.org/install) (extensão do VS Code ou `pip install platformio`, versão 6.2 ou mais nova).
2. Abra a pasta do exemplo (não a raiz do repositório) no VS Code, ou entre nela pelo terminal.
3. Compile, grave e abra o monitor serial, escolhendo a série da sua placa:

```bash
cd capitulo-01/pisca-esp-idf
pio run -e esp32c6                      # só compila
pio run -e esp32c6 -t upload -t monitor # grava e abre o monitor (115200)
```

Os ambientes se chamam `esp32`, `esp32s2`, `esp32s3`, `esp32c2`, `esp32c3`,
`esp32c5`, `esp32c6`, `esp32c61`, `esp32h2` e `esp32p4`. Cada um usa a placa
de desenvolvimento oficial da Espressif para aquela série; se a sua for outra,
troque o `board` no `platformio.ini`.

Os projetos ESP-IDF mantêm a estrutura padrão (`CMakeLists.txt` + `main/`):
também compilam fora do PlatformIO, com `idf.py set-target esp32c6 build`.

## Versões

- Plataforma: [pioarduino](https://github.com/pioarduino/platform-espressif32) `55.03.312`
  (Arduino Core 3.3 sobre ESP-IDF 5.5), fixada em todos os projetos.
- O livro usa o ESP-IDF 6.0 como referência. Os exemplos ESP-IDF também são
  compilados no ESP-IDF 6.0 e 5.5 oficiais; o PlatformIO ainda não tem o 6.0
  em versão estável, por isso aqui eles rodam sobre o 5.5.
- Todos os projetos são compilados em todas as séries listadas a cada
  alteração (GitHub Actions).

## Exemplos

| Capítulo | Projeto | Framework | Séries |
|---|---|---|---|
| 1 · O que é um Microcontrolador | [`pisca-arduino`](capitulo-01/pisca-arduino) | Arduino | todas |
| 1 · O que é um Microcontrolador | [`pisca-esp-idf`](capitulo-01/pisca-esp-idf) | ESP-IDF | todas |
| 2 · Por Dentro de um Computador em um Chip | [`ciclos-arduino`](capitulo-02/ciclos-arduino) | Arduino | todas |
| 2 · Por Dentro de um Computador em um Chip | [`interrupcao-esp-idf`](capitulo-02/interrupcao-esp-idf) | ESP-IDF | ESP32, S2, S3, C3, C6, H2 |
| 2 · Por Dentro de um Computador em um Chip | [`registradores-esp-idf`](capitulo-02/registradores-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 3 · MCU, MPU, SoC e FPGA | [`jitter-esp-idf`](capitulo-03/jitter-esp-idf) | ESP-IDF | todas |
| 4 · A História da Espressif | [`qual-chip-arduino`](capitulo-04/qual-chip-arduino) | Arduino | todas |
| 4 · A História da Espressif | [`qual-chip-esp-idf`](capitulo-04/qual-chip-esp-idf) | ESP-IDF | todas |
| 6 · Os Núcleos: Xtensa e RISC-V | [`fpu-esp-idf`](capitulo-06/fpu-esp-idf) | ESP-IDF | todas |
| 6 · Os Núcleos: Xtensa e RISC-V | [`nucleos-esp-idf`](capitulo-06/nucleos-esp-idf) | ESP-IDF | todas |
| 7 · Memória: ROM, SRAM, Cache, Flash e PSRAM | [`memoria-esp-idf`](capitulo-07/memoria-esp-idf) | ESP-IDF | todas |
| 7 · Memória: ROM, SRAM, Cache, Flash e PSRAM | [`psram-esp-idf`](capitulo-07/psram-esp-idf) | ESP-IDF | ESP32, S2, S3, C5, C61, P4 |
| 7 · Memória: ROM, SRAM, Cache, Flash e PSRAM | [`rtc-contador-esp-idf`](capitulo-07/rtc-contador-esp-idf) | ESP-IDF | ESP32, S2, S3, C3, C5, C6, H2, P4 |
| 8 · Clocks e Relógios | [`clocks-arduino`](capitulo-08/clocks-arduino) | Arduino | todas |
| 8 · Clocks e Relógios | [`clocks-esp-idf`](capitulo-08/clocks-esp-idf) | ESP-IDF | todas |
| 9 · GPIO, IO MUX e Matriz de GPIO | [`gpio-modos-esp-idf`](capitulo-09/gpio-modos-esp-idf) | ESP-IDF | todas |
| 9 · GPIO, IO MUX e Matriz de GPIO | [`modos-gpio-arduino`](capitulo-09/modos-gpio-arduino) | Arduino | todas |
| 10 · Alimentação, Reset e Domínios de Energia | [`motivo-reset-arduino`](capitulo-10/motivo-reset-arduino) | Arduino | todas |
| 10 · Alimentação, Reset e Domínios de Energia | [`motivo-reset-esp-idf`](capitulo-10/motivo-reset-esp-idf) | ESP-IDF | todas |
| 11 · Do Reset ao app_main() | [`identidade-esp-idf`](capitulo-11/identidade-esp-idf) | ESP-IDF | todas |

## Licença

Códigos sob a licença [MIT](LICENSE): use, modifique e distribua à vontade,
inclusive em projetos comerciais. O texto do livro não faz parte deste
repositório e não está sob esta licença.
