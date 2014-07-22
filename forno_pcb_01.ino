// Codigo para controle do Forno Eletrico para PCB.

/* este codigo foi criado por Thiago Kunz, com base em codigos ja existentes.
Os codigos de PID sao provenientes da biblioteca PID v1, o codigo de leitura
e interpretacao do sinal do termistor foi retirado do blog da ADAFRUIT.
O codigo de debounce do botao foi retirado dos exemplos oficiais do arduino.
*/

#include PIDM
  
// variaveis de controle do encoder

 int val; 
 int encoder0PinA = A5;
 int encoder0PinB = A4;
 int encoder0Pos = 0;
 int encoder0PinALast = LOW;
 int n = LOW;
 
 // variaveis de controle do botao
 
 byte pinoBotao = 4;
 boolean estadoBotao = LOW;
 boolean estadoAnteriorBotao = LOW;
 long ultimaVezDebounce = 0;
 int tempoDebounce = 20;
 
 // variaveis do PID e Rele(optoacoplador)
 
 byte pinoRele = 2;
 long mudancaAnteriorRele = 0;
 int timerRele = 2000;
 boolean estadoRele = LOW;
 
 byte buzzerPin = 5; // pino do buzzer para sinais sonoros
 
 
 // variaveis do Termistor - codigo da ADAFRUIT
   
 #define THERMISTORPIN A0  // which analog pin to connect
 #define THERMISTORNOMINAL 100000 // resistance at 25 degrees C
 #define TEMPERATURENOMINAL 25 // temp. for nominal resistance (almost always 25 C)
 #define NUMSAMPLES 2 // how many samples to take and average, more takes longer, but is more 'smooth'
 #define BCOEFFICIENT 4092 // The beta coefficient of the thermistor (usually 3000-4000)
 #define SERIESRESISTOR 4700 // the value of the 'other' resistor
 
int samples[NUMSAMPLES];

 void setup() { 
   
   // setup dos pinos do encoder
   pinMode (encoder0PinA,INPUT);
   digitalWrite(encoder0PinA, HIGH);
   pinMode (encoder0PinB,INPUT);
   digitalWrite(encoder0PinB, HIGH);
   
   // setup do pino do buzzer
   pinMode(buzzerPin, OUTPUT);
   
   // setup do pino do botao
   pinMode(pinoBotao, INPUT);
   
   // setup do rele
   pinMode(pinoRele, OUTPUT);
   digitalWrite(pinoRele, estadoRele);
   
   // setup da comunicacao serial
   Serial.begin (9600);
   
   // setup do termistor
   analogReference(EXTERNAL);
   
 } 

 void loop() {
   
   encoder();
   botao();
   rele();
   funcaoLeituraTermistor();
   delay(1);
   
 }




// funcao de funcionamento do botao
 
 void botao() {
   int leituraBotao = digitalRead(pinoBotao);
   
   if ((millis() - ultimaVezDebounce) > tempoDebounce) {
     
     if (estadoAnteriorBotao != leituraBotao) {
       estadoBotao = leituraBotao;
       ultimaVezDebounce = millis();
       
       if (estadoBotao == HIGH) {
         tone(buzzerPin, 60, 10);
         // colocar aqui outras coisas que voce quer que
         // o seu botao faca.
       }
       
       estadoAnteriorBotao = estadoBotao;
     }
   }
 }
 
 
 
 
 // Funcao de funcionamento do encoder
 
 void encoder() {
   n = digitalRead(encoder0PinA);
   if ((encoder0PinALast == LOW) && (n == HIGH)) {
     if (digitalRead(encoder0PinB) == LOW) {
       encoder0Pos++;
       tone(buzzerPin, 30, 20);
     } else {
       encoder0Pos--;
       tone(buzzerPin, 30, 20);
     }
     Serial.print (encoder0Pos);
     Serial.print ("/");
   } 
   encoder0PinALast = n;
 } 
 
 // Funcao que controla o Rele
 
 void rele() {
   if ((millis() - mudancaAnteriorRele) > timerRele) {
     estadoRele = !estadoRele;
     digitalWrite(pinoRele, estadoRele);
     mudancaAnteriorRele = millis();
   }
 }
 
 // Funcao leitura do termistor - gera o valor de steinhart para a temperatura que sera usada pelo PID.
 
 void funcaoLeituraTermistor() {
   
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  Serial.print("Average analog reading "); 
  Serial.println(average);
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  delay(1);
  }
