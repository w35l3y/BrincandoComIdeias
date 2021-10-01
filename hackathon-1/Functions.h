#include "ConfigFunctions.h"
#include "Utils.h"
#include "Setups.h"

#define COFFEE 300000 // 5 minutos

/**
 * Ação do botão de RESTART
 * 
 * Esta ação permite que eu possa refazer vários testes
 * sem precisar esperar a hora esperada
 * 
 * Redefine as variáveis pro estado inicial e
 * coloca a hora para 5 segundos antes do horário programado
 */
void listenRestart() {
  if (!digitalRead(PIN_OPTION) && current - lastRestart >= 1000) {
    digitalWrite(PIN_RELAY, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Reiniciado      ");

    resetStatus();

    accumulatedOffset -= timeClient.getSeconds() + 5; // 5 seconds before expected time
    accumulatedOffset -= 60 * (timeClient.getMinutes() - expectedMinute);
    accumulatedOffset -= 3600 * (timeClient.getHours() - expectedHour);
    timeClient.setTimeOffset(accumulatedOffset);

    sendBeep(1, 100, 50);
  }
}

/**
 * Atualiza o display com o tempo remanescente para conclusão do preparo.
 * 
 * Fluxos de exceção:
 * - Limpa o trecho do display caso não esteja em preparo
 */
void printRemainingTime() {
  if (makingCoffee) {
    if (current - lastRemainingTime >= 1000) {
      lastRemainingTime = current;
      uint remainingTime = (COFFEE - timeLapsed - current + lastCoffee) / 1000;
      uint remainingMinutes = remainingTime / 60;
      uint remainingSeconds = remainingTime % 60;
      char cstr[5];
      sprintf_P(cstr, PSTR("%02d:%02d"), remainingMinutes, remainingSeconds);
      lcd.setCursor(0, 1);
      lcd.print(cstr);
      remainingTimeCleaned = false;
    }
  } else if (coffeeReady && !remainingTimeCleaned) {
    remainingTimeCleaned = true;
    lcd.setCursor(0, 1);
    lcd.print("     ");
  }
}

// Atualiza a exibição da hora no display
void updateTime() {
  timeClient.update();

  if (current - lastUpdateTime >= 1000) {
    lastUpdateTime = current;
    lcd.setCursor(8, 1);
    lcd.print(timeClient.getFormattedTime());
  }
}

/**
 * Gerencia a situação da jarra.
 * 
 * Ou seja, se está posicionada ou não no local
 * 
 * Fluxos de exceção tratados:
 * - Retirada da jarra com o café sendo preparado
 */
void listenJarState() { // mensagem de baixa prioridade
  if (current - lastCheckJar >= 1000 && jarInPlace == digitalRead(PIN_JAR)) {
    lastCheckJar = current;
    jarInPlace = !jarInPlace;
    if (jarInPlace) {
      lcd.setCursor(0, 0);
      lcd.print("Jarra no local  ");
    } else {
      if (makingCoffee) {
        timeLapsed += current - lastCoffee;
        makingCoffee = false;
        sendBeep(2, 100, 50);
        digitalWrite(PIN_RELAY, LOW);
      }
      lcd.setCursor(0, 0);
      lcd.print("Jarra removida  ");
    }
  }
}
/**
 * Ação que verifica a possibilidade de ligar a cafeteira.
 * 
 * Fluxos de exceção tratados:
 * - Final de semana
 * - Horário correto, mas sem a jarra
 */
void listenStartTime() {
  if (!makingCoffee && !coffeeReady && isShowTime()) {
    if (isWeekend()) {
      if (renderWeekend) {
        renderWeekend = false;
        lcd.setCursor(0, 0);
        lcd.print("Fim de semana   ");
        sendBeep(3, 100, 50);
      }
    } else if (jarInPlace) {
      renderWeekend = true;
      checkJarInPlace = true;
      sendBeep(1, 400, 50);
      makingCoffee = true;
      lastCoffee = current;
      lcd.setCursor(0, 0);
      lcd.print("Esquentando...  ");
      digitalWrite(PIN_RELAY, HIGH);
    } else if (checkJarInPlace) {
      renderWeekend = true;
      checkJarInPlace = false;
      sendBeep(2, 100, 50);
      lcd.setCursor(0, 0);
      lcd.print("Coloque a jarra ");
    }
  }
}

/**
 * Trata o momento que o café está pronto.
 */
void listenEndTime () {
  if (makingCoffee && current - lastCoffee + timeLapsed >= COFFEE) {
    coffeeReady = true;
    makingCoffee = false;
    digitalWrite(PIN_RELAY, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Bom dia!        ");
  } else if (coffeeReady && current - lastCheckJarRemoval >= COFFEE) {
    lastCheckJarRemoval = current;
    sendBeep(6, 400, 9600);  // 1 beep a cada 10 segundos durante 1 minuto
  }
}

/**
 * Trata o momento após o café estar pronto e a jarra ter sido retirada
 */
void listenJarRemoval () {
  if (coffeeReady && !jarInPlace) {
    coffeeReady = false;
    sendBeep(1, 100, 50);
  }
}
