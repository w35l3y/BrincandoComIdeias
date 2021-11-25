/*
Usando a biblioteca LiquidCrystal_I2C na versão 1.1.2 na pasta: ~/Projetos/arduino/libraries/LiquidCrystal_I2C 
Usando a biblioteca Wire na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/Wire 
Usando a biblioteca ESP8266WiFi na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ESP8266WiFi 
Usando a biblioteca DNSServer na versão 1.1.1 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/DNSServer 
Usando a biblioteca ESP8266WebServer na versão 1.0 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ESP8266WebServer 
Usando a biblioteca ESP8266mDNS na versão 1.2 na pasta: ~/.arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ESP8266mDNS 

Bibliotecas baixadas pela IDE
* LiquidCrystal I2C by Frank de Brabander Versão 1.1.2

As demais são nativas
*/
/*
 * 3º HACKATHON CURSODEARDUINO.NET
 * https://brincandocomideias.notion.site/brincandocomideias/3-Hackathon-CursoDeArduino-NET-797b8d537e8447d69fbb8d78b64a0963
 * 
 * Versão de demonstração em um Arduino UNO antes de criar o servidor web: https://wokwi.com/arduino/projects/315150214910444097
 * Não conheço nenhum simulador ESP8266 com Display LCD I2C
 * 
 * Amigo Secreto
 * 
 * NÍVEL 1
 * [OK]  Monte o circuito com um botão e dois leds (vermelho e verde)
 * [OK]  Programe o Arduino para *ligar* o Led Vermelho e *desligar* o **Led Verde ao apertar o PushButton
 *       Obs. Após 5 segundos o Led vermelho deve desligar e o Led verde deve permanecer ligado até que o botão seja apertado novamente.
 * [OK]  Programe o Arduino para *sortear* e *exibir* um número entre 0 e 4 ao apertar o botão
 *       Obs. O sorteio ocorre enquanto o Led vermelho está ligado
 * NÍVEL 2
 * [OK]  Crie um *array* do tipo **String**Nomes[5] com 5 Nomes
 *       Ex. `String Nomes[5] = { "Nome1", "Nome2", "Nome3", "Nome4", "Nome5"};`
 * [OK]  Agora você deve exibir um nome no lugar do número sorteado
 * [OK]  Adicione uma validação para não exibir o mesmo nome do número que está sorteando
 *       Ex. na primeira vez que apertar o botão, não pode exibir o "Nome 1", na segunda vez que apertar o botão, não pode exibir o "Nome 2" etc.
 * NÍVEL 3
 * [OK]  Adicione um *array* de **int** *numSorteado[5]*, e guarde os números sorteados nele.
 * [OK]  Programe para que o Arduino não exiba o mesmo nome duas vezes.
 *       Obs. O sorteio pode, e muito provavelmente vai, repetir os números, mas você só pode exibir os nomes que não foram sorteados.
 * NÍVEL 4
 * [OK]  Adicionar algum Display
 * [OK]  Fazer Efeitos com buzzer
 * [  ]  Salvar o sorteio na memória para conferir depois
 * [OK]  Sorteio WEB
 * [OK]  Possibilidade de ajustar nome dos participates a partir da versão WEB
 * [OK]  Modo aplicativo
 * [OK]  Sorteio de ciclo único na versão WEB
 * 
 * TIPOS DE ALARME
 * 
 * Exibição do sorteio    1 beep curto (led vermelho acende)
 * Fim do sorteio         1 beep curto (led verde acende)
 * Próximo participante   2 beeps curtos (led verde acende)
 * Aguarde próxima etapa  3 beeps curtos (led vermelho pisca no mesmo ritmo)
 * 
 * COMPONENTES UTILIZADOS
 * 1. Wemos D1 Mini
 * 2. Buzzer ativo
 * 3. Push button
 * 4. Display LCD
 * 5. Leds
 * 6. Resistores (220 ohms)
 * 7. Jumpers (macho/macho, macho/fêmea)
 * 8. Protoboard
 * 
 * FUNÇÕES IMPLEMENTADAS
 * 
 * 1. WI-FI Access Point
 *    Cria uma rede WI-FI chamada "Amigo Secreto" sem senha para que os participantes se conectem
 *    O acesso é feito através do IP 192.168.4.1
 * 2. Versão WEB
 *    Os participantes acessam a partir de QUALQUER dispositivo (Smartphone, Notebook, Tablet) com navegador WEB
 *    Compatível com sistemas operacionais mais conhecidos, como: Android, iOS, Windows ou Linux
 * 3. Acesso automático quando conectado
 *    O navegador abre automaticamente na página do sorteio
 *    Compatível apenas com alguns aparelhos
 * 4. Modo aplicativo (smartphones ou tablets)
 *    Atalho do site na tela principal que se assemelha a um aplicativo nativo
 * 5. Sorteio ciclo único
 *    A versão WEB executa sorteio de forma que fecha um único ciclo
 *    Ou seja, ele evita que ocorra, por exemplo, que o 1º sorteie o 2º e o 2º sorteie o 1º
 * 6. Refazer sorteio
 *    Ao final do sorteio, o dispositivo permite efetuar um novo sorteio
*/
//#define DEBUG

#ifdef DEBUG
#define setup_serial() Serial.begin(9600)
#define debugln(A) Serial.println(A)
#define debug(A) Serial.print(A)
#else
#define setup_serial()
#define debugln(A)
#define debug(A)
#endif

#include <LiquidCrystal_I2C.h>
#include <time.h>
#include "Buzzer.h"
#include "PushButton.h"
#include "Led.h"
#include "Utils.h"
#include "Server.h"

void setup()
{
  setup_serial();
  setup_lcd();
  setup_server();

  delay(2000);

  setup_button();
}

void loop()
{
  loop_server();

  button.loop();
  buzzer.loop();
  ledRed.loop();
  //ledGreen.loop();  // não precisa, já que não pisca
}
