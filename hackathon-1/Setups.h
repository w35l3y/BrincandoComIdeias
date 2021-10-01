// Definições dos pinos
#define PIN_JAR D5            // OK - PULL UP
#define PIN_RELAY D8

// Valores padrão das variáveis globais
int accumulatedOffset = -10800;
bool jarInPlace = false;
uint lastUpdateTime = 0;
uint lastRestart = 0;
bool coffeeReady = false;
bool makingCoffee = false;
uint lastCoffee = 0;
bool checkJarInPlace = true;
uint lastCheckJarRemoval = 0;
uint lastCheckJar = 0;
uint timeLapsed = 0;
uint lastRemainingTime = 0;
bool remainingTimeCleaned = false;
bool renderWeekend = true;

// Função para redefinir o estado inicial
void resetStatus() {
  jarInPlace = !digitalRead(PIN_JAR);
  checkJarInPlace = true;

  lastBeep = 0;
  totalBeeps = 0;
  timerOn = 0;
  timerOff = 0;
  beepOn = false;

  lastAnalogRead = 0;
  lastUpdateTime = 0;
  lastCheckJarRemoval = 0;
  lastCheckJar = 0;
  lastRemainingTime = 0;
  remainingTimeCleaned = false;
  renderWeekend = true;

  timeLapsed = 0;

  lastRestart = current;
  coffeeReady = false;
  makingCoffee = false;
  lastCoffee = 0;
}

void setupJar() {
  pinMode(PIN_JAR, INPUT_PULLUP);
}

void setupDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...    ");
}

void setupRelay() {
  pinMode(PIN_RELAY, OUTPUT);
}

void setupBuzzer() {
  pinMode(PIN_BUZZER, OUTPUT);
}

void setupRestart() {
  pinMode(PIN_OPTION, INPUT_PULLUP);
}

void setupLed () {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
}

void setupWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  lcd.setCursor(0, 0);
  lcd.print("Conectando      ");
  int cursor = 10;
  lcd.setCursor(cursor, 0);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (++cursor > 13) {
      cursor = 10;
      lcd.setCursor(cursor, 0);
      lcd.print("   ");
      lcd.setCursor(cursor, 0);
    } else {
      lcd.print(".");
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Conectado!      ");
}

void setupTime() {
  timeClient.begin();
  timeClient.setTimeOffset(accumulatedOffset); // -3 hours
  timeClient.update();
}

void setupMode () {
  if (configMode || !digitalRead(PIN_OPTION)) {
    configMode = true;
    lcd.setCursor(0, 0);
    lcd.print("Config          ");
    delay(1000);
  }
}
