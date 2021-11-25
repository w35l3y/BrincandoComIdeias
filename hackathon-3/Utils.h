LiquidCrystal_I2C lcd(0x27, 16, 2);
// LCD D1 = 5 / D2 = 4

#define PIN_PUSHBUTTON LED_BUILTIN  // D4 = 2 ; 13 = D7
#ifdef ESP8266
#define PIN_LED_GREEN D6  // 12 ok
#define PIN_LED_RED D7  // 13
#define PIN_BUZZER D5 // 14
#else
#define PIN_LED_GREEN 12  // D6 ok
#define PIN_LED_RED 10  // ?
#define PIN_BUZZER 11 // ?
#endif

#define MAX_SORTEADOS 5
#define TEMPO_AMIGO 5000

#define WAIT_MODE 0
#define END_MODE 1
#define START_MODE 2

Buzzer buzzer(PIN_BUZZER);
PushButton button(PIN_PUSHBUTTON);
Led ledGreen(PIN_LED_GREEN);
Led ledRed(PIN_LED_RED);

unsigned mode = START_MODE;
unsigned long lastStarted = 0L;

unsigned int sorteados = 0;
int numSorteados[MAX_SORTEADOS] = { 0, 1, 2, 3, 4 };
String nomes[MAX_SORTEADOS] = { "Nome 1", "Nome 2", "Nome 3", "Nome 4", "Nome 5" };

void setup_lcd() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Amigo Secreto!");
  lcd.setCursor(2,1);
  lcd.print("Hackathon #3");
}

unsigned long rnd(unsigned long min) {
  srand(time(NULL));
  return min + (rand() % (MAX_SORTEADOS - min));
}
