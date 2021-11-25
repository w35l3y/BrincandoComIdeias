/*
 * Esta biblioteca é mais apropriada para controlar pushbutton de pullup.
 * 
 * Controla efeito debounce
 * Orientado a evento
 * Diferencia clique curto do longo
 * Atua com o número de cliques customizado (1, 2, 3+)
*/
#define PUSHBUTTON_RELEASE_MODE 0
#define PUSHBUTTON_PRESS_MODE 1

#define PUSHBUTTON_LONG_PRESS_MS 200

class PushButton {
  private:
    unsigned int pin;
    bool longPress = false;
    unsigned int times = 0;
    unsigned long debounce = 0L;
    void (*callback)();
    unsigned long lastPress = 0;
    unsigned int lastMode = PUSHBUTTON_PRESS_MODE;
    unsigned int timesPressed = 0;

    bool pressed();
  public:
    PushButton(unsigned int pin, bool longPress = false, unsigned long debounce = 15);
    void loop();
    void onPress(unsigned int times, void (*f)());
    void onSinglePress(void (*f)());
    void onDoublePress(void (*f)());
    unsigned long lastPressed();
};

PushButton::PushButton(unsigned int pin, bool longPress, unsigned long debounce) {
  this->pin = pin;
  this->longPress = longPress;
  this->debounce = debounce;
  pinMode(pin, INPUT_PULLUP);
}

unsigned long PushButton::lastPressed() {
  return millis() - this->lastPress;
}

bool PushButton::pressed() {
  return !digitalRead(this->pin);
}

void PushButton::loop() {
  unsigned long current = millis();
  unsigned long diff = current - this->lastPress;
  if (diff >= this->debounce) {
    bool isPressed = this->pressed();
    if (isPressed && this->lastMode == PUSHBUTTON_PRESS_MODE) {
      this->lastPress = current;
      this->lastMode = PUSHBUTTON_RELEASE_MODE;
    } else if (!isPressed && this->lastMode == PUSHBUTTON_RELEASE_MODE) {
      this->lastMode = PUSHBUTTON_PRESS_MODE;
      if ((this->longPress && diff >= PUSHBUTTON_LONG_PRESS_MS) || (!this->longPress && diff < PUSHBUTTON_LONG_PRESS_MS)) {
        ++this->timesPressed;
        if (this->timesPressed == this->times) {
          this->lastPress = current;
          this->callback();
          this->timesPressed = 0;
        }
      } else {
        this->timesPressed = 0;
      }
    }
  }
}

void PushButton::onPress(unsigned int times, void (*f)()) {
  this->times = times;
  this->callback = f;
}

void PushButton::onSinglePress(void (*f)()) {
  this->onPress(1, f);
}

void PushButton::onDoublePress(void (*f)()) {
  this->onPress(2, f);
}
