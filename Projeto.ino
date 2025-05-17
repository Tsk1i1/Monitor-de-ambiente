#include <LiquidCrystal_I2C.h> // Biblioteca para LCD I2C
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real
#include <Wire.h>   // Biblioteca para comunicação I2C
#include "DHT.h"  // Biblioteca para utilizar sensores de temperatura e umidade DHT
#include <EEPROM.h> // Biblioteca para armazenamento não volátil

#define LOG_OPTION 1 // Ativa ou desativa o registro de dados
#define MOSTRAR_MEDIA 1 // Mostra ou não a média da luminosidade na serial
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3

// Configurações do DHT
#define DHTPIN 3 // Pino de leitura do sensor DHT
#define DHTTYPE DHT22 // Tipo do sensor DHT utilizado
DHT dht(DHTPIN, DHTTYPE); // Inicializa o sensor

LiquidCrystal_I2C lcd(0x27,16,2); // Endereço de acesso: 0x3F ou 0x27
RTC_DS1307 RTC;  // Inicializa o módulo de tempo real

// Declara as portas do Arduino
int Lerluz = A0; // Sensor de luz (LDR)
int LuzVerde = 12;
int LuzAmarela = 11;
int LuzVermelha = 10;
int buzzer = 9;

// Configurações para a calibragem
int luzmax = 1023; // Valor inicial máximo de luz
const int intervalo = 10000; // Tempo entre médias (10 segundos)
unsigned long tempoAnterior = 0; // Controle do tempo
const int amostras = 10;     // Número de leituras para média
int somaLuz = 0;
int somaTemp = 0;
int somaUmid = 0;
int contador = 0;

// Configuraçãoes do EEPROM
const int maxRecords = 100; // Número máximo de registros
const int recordSize = 10; // Tamanho de cada registro em bytes, 4 bytes do horario e 3 int's para luminosidade, temperatura e umidade
int startAddress = 0; // Início do endereço de gravação
int endAddress = maxRecords * recordSize;
int currentAddress = 0;

int lastLoggedMinute = -1;

// Criação de caracteres personalizados para o LCD
byte lamp[8] = {
  B01110,  //  ###
  B11111,  // #####
  B11111,  // #####
  B11111,  // #####
  B01110,  //  ###
  B01110,  //  ###
  B00000,  //
  B01110   //  ### 
};

byte term[8] = {
  B01110,  //   #
  B01010,  //  # #
  B01010,  //  # #
  B01010,  //  # #
  B01010,  //  # #
  B11011,  // ## ##
  B11011,  // ## ##
  B01110   //  ###
};

byte gota[8] = {
  B00010,  //    #
  B00100,  //   #
  B01010,  //  # #
  B01010,  //  # #
  B10001,  // #   #
  B10001,  // #   #
  B01110,  //  ###
  B00000   //
};

byte Feliz[8] = {
  B00000,
  B01010,  // Olhos
  B00000,
  B00000,
  B10001,  // Bochechas
  B01110,  // Sorriso
  B00000,
  B00000
};

byte Meh[8] = {
  B00000,
  B01010,  // Olhos
  B00000,
  B00000,
  B00000,
  B01110,  // Boca reta
  B00000,
  B00000
};

byte Triste[8] = {
  B00000,
  B10001,  // Olhos com expressão
  B01010,  // Sobrancelhas inclinadas
  B00000,
  B00000,
  B01110,  // Boca para baixo
  B10001,
  B00000
};

void setup() {
// Declarando saídas e entradas do arduino.
  pinMode(Lerluz, INPUT);
  pinMode(LuzVerde, OUTPUT);
  pinMode(LuzAmarela, OUTPUT);
  pinMode(LuzVermelha, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
//
  Serial.begin(9600);
  dht.begin();
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //RTC.adjust(DateTime(2025,5,17,14,23,00));
  
  EEPROM.begin();
  
// Configurando LCD para mensagens
  lcd.init(); // Define o LCD como 16 colunas e 2 linhas
  lcd.backlight();
  lcd.createChar(1, lamp);
  lcd.createChar(2, term);
  lcd.createChar(3, gota);
  lcd.createChar(4, Feliz);
  lcd.createChar(5, Meh);
  lcd.createChar(6, Triste);

//Tela de boas-vindas animada
  for (int i = 0; i < 6; i++) {
    lcd.print("    Vinheria"); // Exibe texto centralizado
    lcd.setCursor(0,1); // Move para a segunda linha
    lcd.print("    Agnello"); // Exibe Nome da vinheria juntamente dos caracteres personalizados de garrafa e taça
    delay(500);
    lcd.clear(); // Limpa display
    delay(500);
  }
  lcd.print("Seja bem-vindo!"); // Mensagem final de boas-vindas
  delay(2000);
  lcd.clear();
  calibragem(); // Inicia processo de calibragem da luz
  }

void calibragem(){
// Mensagem de auto calibragem
  lcd.setCursor(0, 0);                  // Define o cursor 
  lcd.print("Iniciando");              // Mostra "Iniciando" no LCD
  lcd.setCursor(0, 1);                  
  lcd.print("Auto Calibragem");        // Mostra "Auto Calibragem"
  delay(2000);                          
  lcd.clear();                         // Limpa o display

  lcd.setCursor(0, 0);
  lcd.print("Deixe");                  // Pede ao usuário para deixar a luz ligada
  lcd.setCursor(0, 1);
  lcd.print("a luz ligada");          
  delay(2000);                          // Aguarda 2 segundos
  lcd.clear();                          // Limpa o display

// Auto Calibragem
  for (int j = 0; j <= 5; j++) {        // Repete o processo 6 vezes 
    lcd.setCursor(0, 1);         
    lcd.print("Calibrando");           // Exibe a mensagem de calibragem

    int luzbeta = analogRead(Lerluz); // Lê o valor do sensor LDR
    if (luzbeta < luzmax) {           // Se o valor atual for menor que o anterior
      luzmax = luzbeta;               // Atualiza luzmax com o menor valor (mais luz)
    }
    lcd.setCursor(10, 1);              // Posiciona o cursor para os pontos de carregamento
    for (int k = 0; k < 3; k++) {   // Animação com 3 pontos
        lcd.print(".");              
        delay(500);                   // Pequeno atraso entre os pontos
    }

    lcd.clear();                    // Limpa o display após cada rodada
  }
}

void loop() {

  DateTime now = RTC.now(); // Lê o tempo atual do RTC

  // Aplica o fuso horário UTC-3
  int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
  now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual
  DateTime adjustedTime = DateTime(now); // Tempo ajustado
  
  if (LOG_OPTION) get_log(); // Função de log, se ativada

// Leitura de sensores
  int luz = analogRead(Lerluz);
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  int nivelLuz = map(luz, luzmax, 1023, 100, 0);  // Mapeia o valor da luz para uma escala de 0 a 100%

// Coleta da média de luminosidade em 10s
  int leituraAtualLuz  = nivelLuz;
  int leituraAtualTemp = temperatura;
  int leituraAtualUmid = umidade;
  somaLuz += leituraAtualLuz;
  somaTemp += leituraAtualTemp;
  somaUmid += leituraAtualUmid;
  contador++;

  if (millis() - tempoAnterior >= intervalo) {
    tempoAnterior = millis();
  }
  int mediaLuz = somaLuz / contador;
  int mediaTemp = somaTemp / contador;
  int mediaUmid = somaUmid / contador;

  if (MOSTRAR_MEDIA){
    Serial.println("________________________________________________________________________");
    Serial.println("|         Média dos valores médidos nos ultimos 10 segundos            |");
    Serial.print("       Luminosidade: ");
    Serial.print(mediaLuz);
    Serial.print(" %   Temperatura: ");
    Serial.print(mediaTemp);
    Serial.print("°C   Umidade: ");
    Serial.print(mediaUmid);
    Serial.print("% ");
    Serial.println("________________________________________________________________________");
  }
  // Reset para próxima média
  somaLuz = 0;
  somaTemp = 0;
  somaUmid = 0;
  contador = 0;

  lcd.setCursor(0,0);
  lcd.print("\x01:"); // Ícone de luz
  lcd.print(nivelLuz);
  lcd.print("% ");
  lcd.print("\x02:"); // Ícone de termômetro
  lcd.print(temperatura, 1);
  lcd.print("C ");
  lcd.setCursor(0,1);
  lcd.print("\x03:"); // Ícone de gota
  lcd.print(umidade, 0);
  lcd.print("%");

// Monitoramento da Luz
  if (nivelLuz >= 70){ 
    // Ambiente muito iluminado (>80%) -> LED vermelho e buzzer (Acima de 55 Lux) .
    digitalWrite (LuzVerde, LOW);
    digitalWrite (LuzAmarela, LOW);
    digitalWrite (LuzVermelha, HIGH);
    digitalWrite (buzzer, HIGH);
    lcd.setCursor(7, 1);
    lcd.print("\x01\x06"); // Luz + carinha triste
  }
  if (nivelLuz >= 50 && nivelLuz < 70) {
    // Alerta (70-80% iluminação) -> LED amarelo (Entre 42 e 55).
    digitalWrite (LuzVerde, LOW);
    digitalWrite (LuzAmarela, HIGH);
    digitalWrite (LuzVermelha, LOW);
    digitalWrite (buzzer, LOW);
    lcd.setCursor(7, 1);
    lcd.print("\x01\x05"); // Luz + carinha neutra
  }
  if (nivelLuz < 50){ 
    // Penumbra ideal (<70% iluminação) -> LED verde (Abaixo de 42 lux) 
    digitalWrite (LuzVerde, HIGH);
    digitalWrite (LuzAmarela, LOW);
    digitalWrite (LuzVermelha, LOW);
    digitalWrite (buzzer, LOW);
    lcd.setCursor(7, 1);
    lcd.print("\x01\x04"); // Luz + carinha feliz
  }
  delay(3000);
    
// Monitoramento da Temperatura
  if (temperatura > 16 || temperatura < 10) {
    digitalWrite(LuzVerde, LOW);
    digitalWrite(LuzAmarela, LOW);
    digitalWrite(LuzVermelha, HIGH);
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(10, 1);
    lcd.print("\x02\x06"); // Termômetro + carinha triste
  } else {
    digitalWrite (LuzVerde, HIGH);
    digitalWrite (LuzAmarela, LOW);
    digitalWrite (LuzVermelha, LOW);
    digitalWrite (buzzer, LOW);
    lcd.setCursor(10, 1);
    lcd.print("\x02\x04"); // Termômetro + carinha feliz

  }
  delay(3000);

// Monitoramento da Umidade
  if (umidade < 60 || umidade > 80) {
    digitalWrite (LuzVerde, LOW);
    digitalWrite (LuzAmarela, LOW);
    digitalWrite (LuzVermelha, HIGH);
    digitalWrite (buzzer, HIGH);
    lcd.setCursor(13, 1);
    lcd.print("\x03\x06"); // Gota + carinha triste
  } else {
    digitalWrite (LuzVerde, HIGH);
    digitalWrite (LuzAmarela, LOW);
    digitalWrite (LuzVermelha, LOW);
    digitalWrite (buzzer, LOW);
    lcd.setCursor(13, 1);
    lcd.print("\x03\x04"); // Gota + carinha feliz
  }
  delay(3000);
  lcd.clear();

  if(nivelLuz >=70 || temperatura > 16 || temperatura < 10 ||umidade < 60 || umidade > 80){
    // Se algum dos sensores estiver em nível critico, armazena os dados.
    // Converte valor para armazenamento
    int luzInt = nivelLuz;
    int tempInt = (int)(temperatura * 100);
    int umidInt = (int)(umidade* 100);

    // Salvar os dados na EEPROM
    EEPROM.put(currentAddress, now.unixtime());     // 4 bytes
    EEPROM.put(currentAddress + 4, luzInt);         // 2 bytes
    EEPROM.put(currentAddress + 6, tempInt);        // 2 bytes
    EEPROM.put(currentAddress + 8, umidInt);        // 2 bytes
    getNextAddress(); // Atualiza o endereço para o próximo registro
  }
}


void getNextAddress() {
    currentAddress += recordSize;
    if (currentAddress >= endAddress) {
        currentAddress = 0; // Volta para o começo se atingir o limite
    }
}

void get_log(){
  Serial.println("Dados armazenados no EEPROM:");
  Serial.println("________________________________________________________________________");
  Serial.println("|Horário\t\t\t| Luminosidade | Temperatura |\tUmidade |");
  for (int address = startAddress; address < endAddress; address += recordSize) {
    long timeStamp;
    int tempInt, umidInt, luzInt;

    // Ler dados salvos da EEPROM
    EEPROM.get(address, timeStamp);
    EEPROM.get(address + 4, luzInt);
    EEPROM.get(address + 6, tempInt);
    EEPROM.get(address + 8, umidInt);

    // desconverter os valores
    float temperatura = tempInt / 100.0;
    float umidade = umidInt / 100.0;

    // Verificar se os dados são válidos antes de imprimir
    if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
      DateTime dt = DateTime(timeStamp);
      Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("\t\t\t");
      Serial.print(luzInt);
      Serial.print(" %\t   ");
      Serial.print(temperatura);
      Serial.print(" C\t");
      Serial.print(umidade);
      Serial.println(" %");
      Serial.println("________________________________________________________________________");
    }
  }
}
