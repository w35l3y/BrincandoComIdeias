/*
 * Esta biblioteca é mais apropriada quando você tem um buzzer que precise ligar e desligar repetidas vezes.
 * 
 * Controla chamando as funções tone (ligado) e noTone (desligado)
 * 
 * Próximas evoluções em vista:
 * - Possibilidade de ajustar frequência
 * - Verificar possibilidade para delegar statusOn e statusOff para o 3º parâmetro da função tone
*/
class Buzzer {
  private:
    bool lastState = false;
    unsigned int pin;
    unsigned int times = 0;
    unsigned long statusOn = 0L;
    unsigned long statusOff = 0L;
    unsigned long lastStatus = 0L;
  public:
    Buzzer(unsigned int pin);
    void beep(unsigned int times, unsigned long statusOn = 100, unsigned long statusOff = 100);
    void loop();
};

Buzzer::Buzzer(unsigned int pin) {
  this->pin = pin;
  pinMode(pin, OUTPUT);
}

void Buzzer::beep(unsigned int times, unsigned long statusOn, unsigned long statusOff) {
  this->times = times;
  this->statusOn = statusOn;
  this->statusOff = statusOff;
}

void Buzzer::loop() {
  if (this->times > 0) {
    unsigned long current = millis();
    unsigned long diff = current - this->lastStatus;
    if (!this->lastState && diff >= this->statusOn) {
      tone(this->pin, 400);
      this->lastState = true;
      this->lastStatus = current;
    } else if (this->lastState && diff >= this->statusOff) {
      noTone(this->pin);
      --this->times;
      this->lastState = false;
      this->lastStatus = current;
    }
  }
}
