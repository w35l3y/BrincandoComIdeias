/*
 * Esta biblioteca é mais apropriada quando você tem um atuador que precise ligar e desligar repetidas vezes.
 * 
 * Embora tenha o nome "Led", mas é possível controlar qualquer outro atuador
 * que funcione com sinal digital HIGH (ligado) ou LOW (desligado).
 * Por exemplo, você pode fazer com que um buzzer ativo bipe N vezes usando "blink".
*/
#define LED_BLINK_MODE 0
#define LED_STATIC_MODE 1

class Led {
  private:
    unsigned mode = LED_STATIC_MODE;
    bool lastState = false;
    bool lastStateBeforeBlink = false;
    unsigned int pin;
    unsigned int times = 0;
    unsigned long statusOn = 0L;
    unsigned long statusOff = 0L;
    unsigned long lastStatus = 0L;

    void localTurn(bool state);
  public:
    Led(unsigned int pin);
    void turn(bool state);
    void turnOn();
    void turnOff();
    void blink(unsigned int times = 1, unsigned long statusOn = 100, unsigned long statusOff = 100);
    void loop();
};

Led::Led(unsigned int pin) {
  pinMode(pin, OUTPUT);
  this->pin = pin;
}

void Led::localTurn(bool state) {
  this->lastStatus = millis();
  this->lastState = state;
  digitalWrite(this->pin, state);
}

void Led::turn(bool state) {
  this->mode = LED_STATIC_MODE;
  this->times = 0;

  this->localTurn(state);
}

void Led::turnOn() {
  this->turn(true);
}

void Led::turnOff() {
  this->turn(false);
}

void Led::loop() {
  if (this->times > 0) {
    unsigned long diff = millis() - this->lastStatus;
    if (!this->lastState && diff >= this->statusOn) {
      this->localTurn(true);
    } else if (this->lastState && diff >= this->statusOff) {
      this->localTurn(false);
      --this->times;
    }
  } else if (this->mode == LED_BLINK_MODE) {
    this->turn(this->lastStateBeforeBlink);
  }
}

void Led::blink(unsigned int times, unsigned long statusOn, unsigned long statusOff) {
  this->mode = LED_BLINK_MODE;
  this->lastStateBeforeBlink = this->lastState;

  this->times = times;
  this->statusOn = statusOn;
  this->statusOff = statusOff;

  this->loop(); // executa efeito imediatamente
}
