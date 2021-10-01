#define PIN_BUZZER D6
#define PIN_LED D4            // OK

uint current = 0;
bool configMode = false;

uint lastBeep = 0;
uint totalBeeps = 0;
uint timerOn = 0;
uint timerOff = 0;
bool beepOn = false;

/**
 * Define as configurações de beep para serem executadas posteriormente.
 * 
 * Isso porque durante a mesma execução do loop, pode haver situações
 * mais prioritárias que outras fazendo com que o comportamento do beep
 * seja sobreescrito por algo mais prioritário
 * 
 * @param _totalBeeps   número de beeps
 * @param _timerOn      tempo que cada beep deve permanecer ligado
 * @param _timerOff     tempo entre beeps
*/
void sendBeep (uint _totalBeeps, uint _timerOn, uint _timerOff) {
  totalBeeps = _totalBeeps;
  timerOn = _timerOn;
  timerOff = _timerOff;
}

/**
 * Executa os beeps priopriamente ditos.
 * 
 * Liga e desliga de acordo com as configurações definidas
 */
void sendBeeps () {
  if (totalBeeps > 0) {
    if (!beepOn && current - lastBeep >= timerOff) {
      lastBeep = current;
      digitalWrite(PIN_BUZZER, HIGH);
      digitalWrite(PIN_LED, LOW);
      beepOn = true;
    } else if (beepOn && current - lastBeep >= timerOn) {
      lastBeep = current;
      digitalWrite(PIN_BUZZER, LOW);
      digitalWrite(PIN_LED, HIGH);
      beepOn = false;
      --totalBeeps;
    }
  }
}
