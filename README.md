# 🔥 Detector de Vazamento de Gás — STM32F103C8T6

> Projeto Final — Microcontroladores  
> Universidade Federal do Ceará

---

## Sobre o Projeto

Este projeto implementa um **detector de vazamento de gás** utilizando o sensor **MQ-2** e o microcontrolador **STM32F103C8T6 (Blue Pill)**, programado em **bare metal**.

O sistema monitora continuamente a presença de gás no ambiente e fornece alertas visuais e sonoros em tempo real, além de log serial via UART.

---

## Funcionalidades

- Leitura digital do sensor MQ-2 via GPIO (pino DOUT);
- Filtragem por votação (2 de 3 amostras) para evitar falsos positivos;
- LED verde aceso indicando ambiente sem gás;
- LED vermelho piscando ao detectar gás;
- Buzzer ativo durante o alarme;
- Log serial via USART1 (9600 baud) com mensagens de status;
- Sequência de inicialização com pisca do LED vermelho (3x).

---

## Hardware Utilizado

| Componente | Descrição |
|---|---|
| STM32F103C8T6 | Microcontrolador ARM Cortex-M3 (Blue Pill) |
| Sensor MQ-2 | Sensor de gás (GLP, fumaça, butano, metano, propano, álcool e hidrogênio) |
| LED Verde | Indicador de ambiente limpo |
| LED Vermelho | Indicador de gás detectado |
| Buzzer ativo | Alarme sonoro |
| Resistores | 330Ω para os LEDs |
| Conversor USB-TTL | Comunicação serial com o PC |
| ST-Link V2 | Gravação do firmware |

---

## Pinagem

| Pino STM32 | Função |
|---|---|
| PA0 | DOUT do sensor MQ-2 (entrada) |
| PA2 | LED verde (saída) |
| PA6 | LED vermelho (saída) |
| PA9 | USART1 TX — log serial |
| PA10 | USART1 RX |
| PB1 | Buzzer (saída) |

---

### Gravação na placa

```bash
openocd \
  -f interface/stlink.cfg \
  -f target/stm32f1x.cfg \
  -c "program PROJETOF_gas.elf verify reset exit"
```

---

## Log Serial

Conecte o conversor USB-TTL com **PA9 → TX**, **PA10 → RX**  e **GND → GND**.  
Abra um terminal serial com as configurações:

```
Baud rate : 9600
Data bits : 8
Parity    : None
Stop bits : 1
```

Exemplo de saída:

```
== Sistema de deteccao de gas iniciando ==
Sistema pronto.
[STATUS] Sem gas.
[STATUS] Sem gas.
[ALERTA] Gas detectado!
[STATUS] Gas presente.
[OK] Ambiente limpo.
[STATUS] Sem gas.
```

---

## Equipe

- MAELY VITORIA GOMES LIRA
- MARYANA MORAES SOUSA 
- RONYER LOPES DA SILVA 

**Professor:** LUIS RODOLFO REBOUCAS COUTINHO 
**Disciplina:** Microcontroladores   
**Período:** 2026.1

---

## Licença

Este projeto foi desenvolvido para fins acadêmicos.
