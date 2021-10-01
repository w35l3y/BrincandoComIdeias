#include <EEPROM.h>
#include "ConfigLcd.h"
#include "ConfigBuzzer.h"

#define PIN_POTENTIOMETER A0  // OK
#define PIN_OPTION D7         // OK - PULL UP

// Definições da situação do botão de configuração
#define BUTTON_END_MODE -2
#define BUTTON_READY_MODE -1
#define BUTTON_WAIT_MODE 0
#define BUTTON_SHORT_MODE 1
#define BUTTON_LONG_MODE 2

// Modos de configuração
#define CONFIG_HOUR 0
#define CONFIG_MINUTE 1
#define CONFIG_END 2

struct customTime {
  uint hour;
  uint minute;
} storedTime = { 7, 0 };

uint currentStateConfig = CONFIG_HOUR;
int buttonConfigMode = BUTTON_END_MODE;
uint updateDisplayConfigMode = true;
uint expectedHour = storedTime.hour;
uint expectedMinute = storedTime.minute;
uint lastButtonPress = 0;
uint lastAnalogRead = 0;

bool isGarbage () {
  return storedTime.hour > 23 || storedTime.minute > 59;
}

void setupStoredData () {
  EEPROM.begin(sizeof(customTime));
  EEPROM.get(0, storedTime);

  configMode = isGarbage();

  if (configMode) {
    storedTime.hour = 7;
    storedTime.minute = 0;
  }
  
  expectedHour = storedTime.hour;
  expectedMinute = storedTime.minute;
}

void saveExpectedTime(uint hour, uint minute) {
  expectedHour = storedTime.hour = hour;
  expectedMinute = storedTime.minute = minute;
  EEPROM.put(0, storedTime);
  EEPROM.commit();
}

/**
 * Atualiza o display com os estados relativos ao processo de configuração.
 * 
 * Manter o botão CONFIGURAÇÃO apertado durante a conexão com WiFi
 * Se já tiver conectado, precisa resetar o arduino
 */
void listenConfigModeDisplay () {
  if (!updateDisplayConfigMode) {
    return;
  }
  updateDisplayConfigMode = false;

  lcd.setCursor(0, 0);
  char cstr[16];
  sprintf_P(cstr, PSTR("Hora:      %02d:%02d"), expectedHour, expectedMinute);
  lcd.print(cstr);
  lcd.setCursor(11, 1);

  if (currentStateConfig == CONFIG_HOUR) {  // configura hora
    lcd.print("^^   ");
  } else if (currentStateConfig == CONFIG_MINUTE) { // configura minuto
    lcd.print("   ^^");
  } else if (currentStateConfig == CONFIG_END) {
    lcd.print("     ");
  }
}

/**
 * Gerencia o estado do botão de configuração
 * 
 * A ordem e definição dos estados é a seguinte:
 * Ready  Estado inicial do botão não pressionado
 * Wait   Tempo mínimo de espera entre uma função e outra
 *        Previne que o botão já seja reconhecido por outra função
 * Short  Identificado um clique curto
 * Long   Identificado um clique longo
 * End    Estado final após o botão ter sido acionado e liberado
 *        Este estado é BEM curto, só até detectar a liberação do botão
 */
void listenConfigModeButton () {
  bool pressed = !digitalRead(PIN_OPTION);

  if (buttonConfigMode == BUTTON_END_MODE && !pressed) {
    lastButtonPress = current;
    buttonConfigMode = BUTTON_READY_MODE;
  } else if (buttonConfigMode == BUTTON_READY_MODE && current - lastButtonPress >= 200 && pressed) {
    lastButtonPress = current;
    buttonConfigMode = BUTTON_WAIT_MODE;
  } else if (buttonConfigMode == BUTTON_WAIT_MODE && current - lastButtonPress >= 1000 && pressed) {
    lastButtonPress = current;
    buttonConfigMode = BUTTON_LONG_MODE;
  } else if (buttonConfigMode == BUTTON_WAIT_MODE && current - lastButtonPress >= 50 && !pressed) {
    lastButtonPress = current;
    buttonConfigMode = BUTTON_SHORT_MODE;
  }
}

void readFromPotentiometer () {
  int value = map(analogRead(PIN_POTENTIOMETER), 14, 1024, -2, 2);
  if (value && current - lastAnalogRead >= 750) {
    lastAnalogRead = current;
    updateDisplayConfigMode = true;
    if (currentStateConfig == CONFIG_HOUR) {
      sendBeep(abs(value), 100, 50);
      expectedHour = (24 + expectedHour + value) % 24;
    } else if (currentStateConfig == CONFIG_MINUTE) {
      sendBeep(abs(value), 100, 50);
      expectedMinute = (60 + expectedMinute + value) % 60;
    }
  }
}
void readFromSerial () {
  if (Serial.available()) {
    String textStr = Serial.readString();
    const char* text = textStr.c_str();
    Serial.print(textStr);
    int hour = 7;
    int minute = 0;
    if (sscanf(text, "%d:%d", &hour, &minute) != 2) {
      int valor = 0;
      if (sscanf(text, "%d", &valor) == 1) {
        if (currentStateConfig == CONFIG_HOUR) {
          expectedHour = valor % 24;
        } else if (currentStateConfig == CONFIG_MINUTE) {
          expectedMinute = valor % 60;
        }

        buttonConfigMode = BUTTON_LONG_MODE;
      }
    } else {
      expectedHour = hour % 24;
      expectedMinute = minute % 60;

      buttonConfigMode = BUTTON_LONG_MODE;
      currentStateConfig = CONFIG_HOUR;
    }
  }
}

/**
 * Executa as ações relativas as configurações
 * 
 * Clique simples   incrementa o valor da configuração atual (hora ou minuto)
 * Clique longo     passa para a próxima função
 * 
 * Se a ação for feita através do potenciômetro,
 * é possível incrementar ou decrementar o valor da configuração atual (hora ou minuto)
 */
void listenConfigModeAction () {
  if (buttonConfigMode == BUTTON_SHORT_MODE) {
    if (currentStateConfig == CONFIG_HOUR) {
      sendBeep(1, 100, 50);
      updateDisplayConfigMode = true;
      expectedHour = (expectedHour + 1) % 24;
    } else if (currentStateConfig == CONFIG_MINUTE) {
      sendBeep(1, 100, 50);
      updateDisplayConfigMode = true;
      expectedMinute = (expectedMinute + 1) % 60;
    }
    buttonConfigMode = BUTTON_READY_MODE;
  } else if (buttonConfigMode == BUTTON_LONG_MODE) {
    sendBeep(1, 400, 50);
    updateDisplayConfigMode = true;
    ++currentStateConfig;
    buttonConfigMode = BUTTON_END_MODE;
  } else if (currentStateConfig == CONFIG_END && buttonConfigMode == BUTTON_READY_MODE) {
    saveExpectedTime(expectedHour, expectedMinute);
    configMode = false;
  } else if (currentStateConfig == CONFIG_HOUR || currentStateConfig == CONFIG_MINUTE) {
    readFromPotentiometer();
    readFromSerial();
  }
}
