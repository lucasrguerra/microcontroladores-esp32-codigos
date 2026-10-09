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
| 12 · ESP32: o Clássico | [`dac-rampa-esp-idf`](capitulo-12/dac-rampa-esp-idf) | ESP-IDF | ESP32, S2 |
| 12 · ESP32: o Clássico | [`rampa-arduino`](capitulo-12/rampa-arduino) | Arduino | ESP32 |
| 12 · ESP32: o Clássico | [`revisao-chip-arduino`](capitulo-12/revisao-chip-arduino) | Arduino | ESP32 |
| 12 · ESP32: o Clássico | [`revisao-chip-esp-idf`](capitulo-12/revisao-chip-esp-idf) | ESP-IDF | ESP32 |
| 13 · ESP32-S2 | [`teclado-usb-arduino`](capitulo-13/teclado-usb-arduino) | Arduino | S2, S3 |
| 13 · ESP32-S2 | [`usb-eco-esp-idf`](capitulo-13/usb-eco-esp-idf) | ESP-IDF | S2, S3 |
| 14 · ESP32-S3 e ESP32-S31 | [`dsp-vetor-esp-idf`](capitulo-14/dsp-vetor-esp-idf) | ESP-IDF | ESP32, S3, C3 |
| 14 · ESP32-S3 e ESP32-S31 | [`quadro-psram-arduino`](capitulo-14/quadro-psram-arduino) | Arduino | ESP32, S3 |
| 15 · ESP32-C2 e ESP32-C3 | [`despertares-arduino`](capitulo-15/despertares-arduino) | Arduino | C3 |
| 15 · ESP32-C2 e ESP32-C3 | [`despertares-esp-idf`](capitulo-15/despertares-esp-idf) | ESP-IDF | C2, C3 |
| 16 · ESP32-C5, ESP32-C6 e ESP32-C61 | [`varredura-arduino`](capitulo-16/varredura-arduino) | Arduino | C5, C6 |
| 16 · ESP32-C5, ESP32-C6 e ESP32-C61 | [`varredura-wifi6-esp-idf`](capitulo-16/varredura-wifi6-esp-idf) | ESP-IDF | C3, C5, C6, C61 |
| 17 · ESP32-H2 e ESP32-H4 | [`farol-termometro-arduino`](capitulo-17/farol-termometro-arduino) | Arduino | C6, H2 |
| 17 · ESP32-H2 e ESP32-H4 | [`ruido-802154-esp-idf`](capitulo-17/ruido-802154-esp-idf) | ESP-IDF | C5, C6, H2 |
| 18 · ESP32-P4 | [`ethernet-p4-arduino`](capitulo-18/ethernet-p4-arduino) | Arduino | P4 |
| 18 · ESP32-P4 | [`jpeg-p4-esp-idf`](capitulo-18/jpeg-p4-esp-idf) | ESP-IDF | P4 |
| 19 · Chips, Módulos e Placas | [`confere-modulo-arduino`](capitulo-19/confere-modulo-arduino) | Arduino | ESP32, S3, C3, C6 |
| 19 · Chips, Módulos e Placas | [`inspecao-esp-idf`](capitulo-19/inspecao-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 20 · Como Escolher o ESP32 Certo | [`capacidades-esp-idf`](capitulo-20/capacidades-esp-idf) | ESP-IDF | todas |
| 21 · O Circuito Mínimo | [`bancada-esp-idf`](capitulo-21/bancada-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 22 · Alimentação: Reguladores, Baterias e Proteção | [`bateria-arduino`](capitulo-22/bateria-arduino) | Arduino | ESP32, S3, C3, C6 |
| 22 · Alimentação: Reguladores, Baterias e Proteção | [`bateria-esp-idf`](capitulo-22/bateria-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 23 · USB: Ponte Serial e USB Nativo | [`usj-eco-esp-idf`](capitulo-23/usj-eco-esp-idf) | ESP-IDF | S3, C3, C6 |
| 24 · RF e Antena | [`varre-rssi-arduino`](capitulo-24/varre-rssi-arduino) | Arduino | ESP32, S3, C3, C6 |
| 24 · RF e Antena | [`varre-rssi-esp-idf`](capitulo-24/varre-rssi-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 25 · Erros Clássicos de Projeto | [`auditoria-pinos-esp-idf`](capitulo-25/auditoria-pinos-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 26 · Primeiros Passos com Arduino | [`pisca-serial-arduino`](capitulo-26/pisca-serial-arduino) | Arduino | ESP32, S3, C3, C6 |
| 26 · Primeiros Passos com Arduino | [`por-tras-arduino`](capitulo-26/por-tras-arduino) | Arduino | ESP32, S3, C3, C6 |
| 27 · Instalando o ESP-IDF | [`ola-esp-idf`](capitulo-27/ola-esp-idf) | ESP-IDF | todas |
| 28 · Anatomia de um Projeto ESP-IDF | [`anatomia-esp-idf`](capitulo-28/anatomia-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 29 · Flash, Partições e Ferramentas | [`particoes-esp-idf`](capitulo-29/particoes-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 30 · Depuração | [`pane-esp-idf`](capitulo-30/pane-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 31 · GPIO e Interrupções | [`botao-isr-esp-idf`](capitulo-31/botao-isr-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 31 · GPIO e Interrupções | [`gpio-dedicado-esp-idf`](capitulo-31/gpio-dedicado-esp-idf) | ESP-IDF | S3, C3, C6 |
| 31 · GPIO e Interrupções | [`hold-sono-esp-idf`](capitulo-31/hold-sono-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 31 · GPIO e Interrupções | [`repique-botao-arduino`](capitulo-31/repique-botao-arduino) | Arduino | ESP32, S3, C3, C6 |
| 32 · ADC, DAC e Sensores Internos | [`adc-dma-esp-idf`](capitulo-32/adc-dma-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 32 · ADC, DAC e Sensores Internos | [`dac-ondas-esp-idf`](capitulo-32/dac-ondas-esp-idf) | ESP-IDF | ESP32, S2 |
| 32 · ADC, DAC e Sensores Internos | [`leitura-continua-arduino`](capitulo-32/leitura-continua-arduino) | Arduino | ESP32, S3, C3, C6 |
| 32 · ADC, DAC e Sensores Internos | [`temperatura-esp-idf`](capitulo-32/temperatura-esp-idf) | ESP-IDF | S3, C3, C6 |
| 33 · PWM: LEDC e MCPWM | [`ledc-fade-esp-idf`](capitulo-33/ledc-fade-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 33 · PWM: LEDC e MCPWM | [`mede-pulso-esp-idf`](capitulo-33/mede-pulso-esp-idf) | ESP-IDF | ESP32, S3, C6 |
| 33 · PWM: LEDC e MCPWM | [`meia-ponte-esp-idf`](capitulo-33/meia-ponte-esp-idf) | ESP-IDF | ESP32, S3, C6 |
| 33 · PWM: LEDC e MCPWM | [`servo-led-arduino`](capitulo-33/servo-led-arduino) | Arduino | ESP32, S3, C3, C6 |
| 34 · Timers e Watchdogs | [`relogios-esp-idf`](capitulo-34/relogios-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 34 · Timers e Watchdogs | [`timer-hw-arduino`](capitulo-34/timer-hw-arduino) | Arduino | ESP32, S3, C3, C6 |
| 34 · Timers e Watchdogs | [`vigia-esp-idf`](capitulo-34/vigia-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 35 · RMT | [`fita-led-esp-idf`](capitulo-35/fita-led-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 35 · RMT | [`fita-rmt-arduino`](capitulo-35/fita-rmt-arduino) | Arduino | ESP32, S3, C3, C6 |
| 35 · RMT | [`rmt-eco-esp-idf`](capitulo-35/rmt-eco-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 36 · UART | [`ponte-serial-arduino`](capitulo-36/ponte-serial-arduino) | Arduino | ESP32, S3, C3, C6 |
| 36 · UART | [`rs485-esp-idf`](capitulo-36/rs485-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 36 · UART | [`uart-linhas-esp-idf`](capitulo-36/uart-linhas-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 37 · I²C | [`i2c-sensor-esp-idf`](capitulo-37/i2c-sensor-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 37 · I²C | [`varre-i2c-arduino`](capitulo-37/varre-i2c-arduino) | Arduino | ESP32, S3, C3, C6 |
| 38 · SPI | [`id-flash-arduino`](capitulo-38/id-flash-arduino) | Arduino | ESP32, S3, C3, C6 |
| 38 · SPI | [`spi-dma-eco-esp-idf`](capitulo-38/spi-dma-eco-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 38 · SPI | [`spi-flash-id-esp-idf`](capitulo-38/spi-flash-id-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 39 · I²S e PDM | [`mic-pdm-esp-idf`](capitulo-39/mic-pdm-esp-idf) | ESP-IDF | ESP32, S3 |
| 39 · I²S e PDM | [`nivel-microfone-arduino`](capitulo-39/nivel-microfone-arduino) | Arduino | ESP32, S3, C3, C6 |
| 39 · I²S e PDM | [`tom-i2s-esp-idf`](capitulo-39/tom-i2s-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 40 · TWAI: CAN | [`can-auto-teste-arduino`](capitulo-40/can-auto-teste-arduino) | Arduino | ESP32, S3, C3, C6 |
| 40 · TWAI: CAN | [`twai-no-esp-idf`](capitulo-40/twai-no-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 41 · Touch, PCNT, SDM e Outros | [`encoder-pcnt-esp-idf`](capitulo-41/encoder-pcnt-esp-idf) | ESP-IDF | ESP32, S3, C6 |
| 41 · Touch, PCNT, SDM e Outros | [`etm-onda-esp-idf`](capitulo-41/etm-onda-esp-idf) | ESP-IDF | C6 |
| 41 · Touch, PCNT, SDM e Outros | [`sdm-led-esp-idf`](capitulo-41/sdm-led-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 41 · Touch, PCNT, SDM e Outros | [`toque-led-arduino`](capitulo-41/toque-led-arduino) | Arduino | ESP32, S3 |
| 42 · Coprocessadores de Baixa Potência | [`lp-vigia-esp-idf`](capitulo-42/lp-vigia-esp-idf) | ESP-IDF | C5, C6, P4 |
| 42 · Coprocessadores de Baixa Potência | [`ulp-fsm-conta-esp-idf`](capitulo-42/ulp-fsm-conta-esp-idf) | ESP-IDF | ESP32 |
| 43 · FreeRTOS no ESP32 | [`pilha-esp-idf`](capitulo-43/pilha-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 43 · FreeRTOS no ESP32 | [`tarefas-esp-idf`](capitulo-43/tarefas-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 43 · FreeRTOS no ESP32 | [`tres-tarefas-arduino`](capitulo-43/tres-tarefas-arduino) | Arduino | ESP32, S3, C3, C6 |
| 44 · Comunicação entre Tarefas | [`coordena-esp-idf`](capitulo-44/coordena-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 44 · Comunicação entre Tarefas | [`corrida-esp-idf`](capitulo-44/corrida-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 44 · Comunicação entre Tarefas | [`fila-sensor-arduino`](capitulo-44/fila-sensor-arduino) | Arduino | ESP32, S3, C3, C6 |
| 45 · Memória Dinâmica | [`corrompe-esp-idf`](capitulo-45/corrompe-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 45 · Memória Dinâmica | [`fragmenta-esp-idf`](capitulo-45/fragmenta-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 45 · Memória Dinâmica | [`memoria-arduino-arduino`](capitulo-45/memoria-arduino-arduino) | Arduino | ESP32, S3, C3, C6 |
| 45 · Memória Dinâmica | [`vazamento-esp-idf`](capitulo-45/vazamento-esp-idf) | ESP-IDF | ESP32, S3 |
| 46 · Armazenamento | [`cartao-sd-esp-idf`](capitulo-46/cartao-sd-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 46 · Armazenamento | [`nvs-boot-esp-idf`](capitulo-46/nvs-boot-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 46 · Armazenamento | [`persistencia-arduino`](capitulo-46/persistencia-arduino) | Arduino | ESP32, S3, C3, C6 |
| 46 · Armazenamento | [`registro-fat-esp-idf`](capitulo-46/registro-fat-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 47 · Eventos, Logs e Tratamento de Erros | [`erros-esp-idf`](capitulo-47/erros-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 47 · Eventos, Logs e Tratamento de Erros | [`eventos-esp-idf`](capitulo-47/eventos-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 47 · Eventos, Logs e Tratamento de Erros | [`falhas-seguidas-arduino`](capitulo-47/falhas-seguidas-arduino) | Arduino | ESP32, S3, C3, C6 |
| 48 · Componentes e Component Manager | [`config-json-esp-idf`](capitulo-48/config-json-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 48 · Componentes e Component Manager | [`media-movel-esp-idf`](capitulo-48/media-movel-esp-idf) | ESP-IDF | ESP32, S3, C3, C6 |
| 48 · Componentes e Component Manager | [`misto-esp-idf`](capitulo-48/misto-esp-idf) | ESP-IDF | ESP32, S3, C3 |

## Licença

Códigos sob a licença [MIT](LICENSE): use, modifique e distribua à vontade,
inclusive em projetos comerciais. O texto do livro não faz parte deste
repositório e não está sob esta licença.
