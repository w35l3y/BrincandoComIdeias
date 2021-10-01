/*
Usando a biblioteca Wire na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/libraries/Wire 
Usando a biblioteca LiquidCrystal_I2C na versão 1.1.2 na pasta: ~/Projetos/arduino/libraries/LiquidCrystal_I2C 
Usando a biblioteca ESP8266WiFi na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/libraries/ESP8266WiFi 
Usando a biblioteca NTPClient na versão 3.2.0 na pasta: ~/Projetos/arduino/libraries/NTPClient 
Usando a biblioteca EEPROM na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/libraries/EEPROM

Bibliotecas baixadas pela IDE
* LiquidCrystal I2C by Frank de Brabander Versão 1.1.2
* NTPClient by Fabrice Weinberg Versão 3.2.0

As demais são nativas

ATENÇÃO: Não esquecer de ajustar as informações do Wifi no arquivo "WifiConfig.h"
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define DEBUG

#include "WifiConfig.h"
#include "Functions.h"

/*
 * 1º HACKATHON CURSODEARDUINO.NET
 * https://quilled-canary-4b3.notion.site/1-Hackathon-CursoDeArduino-NET-6db064e90d9a43b7b637804cbf2d2304
 * 
 * NÍVEL 1
 * [OK] Monte o circuito para utilizar o módulo rele para ligar/desligar a cafeteira
 * [OK] Programe o Arduino para ligar a cafeteira por 5 minutos e depois desligar
 * [OK] Programe o Arduino para repetir esse processo apenas a cada 23h55min
 *      Reprogramado no NÍVEL 3
 * NÍVEL 2
 * [OK] Adicione um buzzer no circuito
 * [OK] Programe para o buzzer ligar por 1 minuto assim que o café estiver pronto
 * [OK] Melhore esse alerta sonoro para um toque intermitente
 *      1 beep a cada 10 segundos durante 1 minuto
 * NÍVEL 3
 * [~~] Adicione o RTC ao circuito
 *      Não tenho o módulo. Substitui por comunicação com servidor NTP por WiFi
 * [OK] Mude a programação para ligar a cafeteira sempre no mesmo horário
 *      Pode ser ativado a qualquer momento entre 07:00 e 07:05
 * [OK] Programe para que nos finais de semana a cafeteira não ligue
 * 
 * NÍVEL 4
 * [OK] Adicionar um sensor para verificar se a jarra está posicionada na cafeiteira
 * [OK] Programe para desligar a cafeteira se a jarra for removida
 *      Antes:   Se der o horário, é dado um aviso para colocar a jarra e a cafeteira não liga
 *               Se der 07:05 e a jarra não for colocada, a programação só ligará no próximo dia útil
 *      Durante: Pausa a cafeteira até que a jarra seja recolocada
 *               Se der 07:05 e a jarra não for recolocada, a programação só ligará no próximo dia útil
 *      Depois:  Encerra o alarme de café pronto e fica pronto para o próximo dia útil
 * [OK] O buzzer deve tocar novamente por 1 minutos a cada 5 minutos, até que nosso Arduino saiba que acordamos
 * 
 * NÍVEL 5
 * [OK] Display LCD
 * [OK] Comunicação com servidor NTP por WiFi (por falta do módulo RTC)
 * [OK] Salva horário configurado na EEPROM
 * [OK] Configuração do horário para ligar cafeteira (botão multifução)
 * [OK] Configuração do horário para ligar cafeteira (potenciômetro)
 *      5 níveis:
 *      - Decrementa 2
 *      - Decrementa 1
 *      - Não faz nada
 *      - Incrementa 1
 *      - Incrementa 2
 * [OK] Configuração do horário para ligar cafeteira (serial)
 *      Formatos esperados:
 *      - HH:MM (ajusta hora e minuto)
 *      - D (ajusta valor da configuração atual, seja hora ou minuto)
 *      Se os valores informados forem muito maiores do que 
 * 
 * TIPOS DE ALARME
 * 
 * Restart                 1 beep curto
 * Início com jarra        1 beep longo
 * Início sem jarra        2 beeps curtos
 * Jarra removida durante  2 beeps curtos
 * Término com jarra       6 beeps longos espaçados (repete após 5 minutos até a jarra ser removida)
 * Término sem jarra       1 beep curto
 * Fim de semana           3 beeps curtos
 * Muda configuração       1 beep longo
 * 
 * COMPONENTES UTILIZADOS
 * 
 * 1. Wemos D1 Mini
 * 2. Módulo relé 1 canal
 * 3. Buzzer ativo
 * 4. Push button (restart/configuração)
 * 5. Push button (detectar jarra)
 * 6. Display LCD
 * 7. Led
 * 8. Resistor (220 ohms)
 * 9. Potenciômetro (B10K)
 * 10. Jumpers (macho/macho, macho/fêmea)
 * 
 * FUNÇÕES IMPLEMENTADAS
 * 
 * 1. RESTART
 *    Simula o horário 06:55:55 para poder testar a funcionalidade diversas vezes
 * 2. Ligação inteligente
 *    Liga no horário caso a jarra esteja posicionada e avisa caso não esteja
 * 3. Retomada inteligente
 *    Recomeça de onde parou caso a jarra seja removida e recolocada em tempo hábil
 * 4. Alarme inteligente
 *    Avisa por 1 minuto a cada 5 minutos que o café está pronto até que a jarra seja retirada
 * 5. Configuração horário inicial
 *    1. Manter botão OPTION apertado enquanto estiver conectando na internet (boot)
 *    2. Clique curto incrementa valor
 *    3. Clique longo muda função
 *    Obs.: Também é possível fazer o ajuste pelo potenciômetro ou pelo serial.
*/

void setup() {
  Serial.begin(115200);

  setupStoredData();
  setupLed();
  setupRelay();
  setupBuzzer();
  setupRestart();
  setupDisplay();
  setupWifi();
  setupTime();
  setupJar();
  setupMode();

  resetStatus();
}

void loop() {
  current = millis();

  if (configMode) {
    listenConfigModeDisplay();
    listenConfigModeButton();
    listenConfigModeAction();
  } else {
    listenRestart();
    printRemainingTime();
  
    updateTime();

    listenJarState();
    listenStartTime();
    listenEndTime();
    listenJarRemoval();
  }

  sendBeeps();
}
