# Monitor de Ambiente 

Este projeto utiliza um Arduino, um sensor LDR para luminosidade, sensor DHT22 para temperatura e umidade, um módulo RTC para marcação de tempo, um display LCD I2C 16x2 com ícones personalizados, LEDs coloridos e um buzzer para monitorar e registrar as condições ambientais em uma vinheria ou ambiente controlado. O sistema exibe dados em tempo real, sinaliza três níveis de luminosidade e grava automaticamente os registros na EEPROM.

## Componentes

- **Arduino Uno (ou compatível)**
- **Sensor LDR** (Light Dependent Resistor)
- **Sensor DHT22** (Temperatura e Umidade)
- **Módulo RTC DS1307** (Relógio de Tempo Real)
- **Display LCD 16x2 com interface I2C**
- **LEDs**: 1 verde, 1 amarelo, 1 vermelho
- **Buzzer** piezoelétrico
- **Resistores**: 5x 220 Ω (para LEDs e sensor)
- **Cabos jumper**
- **Protoboard**

## Diagrama de Ligação

| Componente       | Pino Arduino / Módulo |
|------------------|----------------------|
| LDR              | A0                   |
| DHT22 Data       | 8                    |
| LED Verde        | 12                   |
| LED Amarelo      | 11                   |
| LED Vermelho     | 10                   |
| Buzzer           | 9                    |
| LCD I2C SDA      | A4.2 (Arduino Uno)      |
| LCD I2C SCL      | A5.2 (Arduino Uno)      |
| RTC I2C SDA      | A4 (Arduino Uno)      |
| RTC I2C SCL      | A5 (Arduino Uno)      |

> **Obs.:** Use resistores de 220 Ω em série com cada LED. O LCD e o RTC compartilham os pinos I2C (SDA, SCL).

## Instalação de Software

1. Instale a [IDE do Arduino](https://www.arduino.cc/en/software).
2. Instale as bibliotecas abaixo pelo Gerenciador de Bibliotecas da IDE:
   - `LiquidCrystal_I2C`
   - `RTClib`
   - `DHT sensor library`
3. Copie o arquivo `monitor_ambiental.ino` para a pasta de sketches do Arduino.
4. Selecione a placa correta em **Ferramentas > Placa** e a porta serial em **Ferramentas > Porta**.

##  Outras Bibliotecas Utilizadas

Este projeto utiliza as seguintes bibliotecas do Arduino:

- **EEPROM**: Permite armazenar e recuperar dados na memória interna do Arduino, útil para salvar configurações ou estados entre reinícios.
- **Wire**: Implementa comunicação I2C para se comunicar com dispositivos como sensores ou displays compatíveis com esse protocolo.

> Essas bibliotecas já estão inclusas na IDE padrão do Arduino, então não é necessário instalá-las manualmente.

## Uso

1. Faça o upload do sketch para o Arduino.
2. O sistema realizará uma auto-calibração inicial da luz ambiente (mantenha a luz ligada).
3. O LCD mostrará luminosidade em %, temperatura (°C), umidade (%) e um emoji indicando as condições do ambiente.
4. LEDs e buzzer funcionam conforme:
   - **Luminosidade alta (>70%)**: LED vermelho aceso + buzzer ligado + emoji triste.
   - **Luminosidade média (50–70%)**: LED amarelo aceso + emoji neutro.
   - **Luminosidade baixa (<50%)**: LED verde aceso + emoji feliz.
   - **Temperatura crítica(<10 & >16 °C)**: LED vermelho aceso+ buzzer ligado + emoji triste.
   - **Temperatura adequada(10-16 °C)**: LED verde aceso + emoji feliz.
   - **Umidade crítica (<60% & >80%)**: LED vermelho aceso + buzzer ligado + emoji triste.
   - **Umidade adequada(60-80)**: LED verde aceso + emoji feliz.
5. A cada minuto, o sistema grava na EEPROM o registro com horário, luminosidade, temperatura e umidade.

## Calibração da Luminosidade

O sistema identifica o valor máximo de luminosidade na auto-calibração e usa este valor para mapear a porcentagem da luz ambiente:

```cpp
// Exemplo de calibração
int luz = analogRead(A0);
// Mapeia do valor máximo captado a 1023 para 100–0%
int nivelLuz = map(luz, luzmax, 1023, 100, 0);
```

A calibração ocorre automaticamente na inicialização.

## Estrutura do Código

- **setup()**: Inicializa sensores, LCD, RTC, LEDs, buzzer e realiza auto-calibração.
- **loop()**: Lê sensores, atualiza display com dados e emoji, aciona LEDs e buzzer, e grava dados na EEPROM.
- **calibragem**: Lê o sensor LDR e armazena o valor de maior luminosidade para o funcionamento do sistema.
- **getNextAddress**: Atualiza endereços para registros na EEPROM.
- **get_log**: Lê dados na EEPROM e mostra no dispositivo de comunicação serial. 

## Simulação do Código
Simule esse projeto em https://wokwi.com/projects/430357110730052609

## Colaboradores

- Cesar Aaron Herrera
- Kaue Soares Madarazzo
- Rafael Seiji Aoke Arakaki
- Rafael Yuji Nakaya
- Nicolas Mendes dos Santos

## Agradecimentos

- Professor Nome do Professor (Disciplina: Edge Computing and Computer Systems, FIAP)

> **Observação:** Este projeto foi desenvolvido como parte da disciplina *Edge Computing and Computer Systems* na FIAP.
