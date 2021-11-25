/*
 * Esta biblioteca foi inicialmente desenvolvida para controlar uma pista de corrida de leds
 * Percebi que algumas lógicas poderiam ser mantidas
 * Ela deverá sofrer ainda muitas modificações antes de chegar numa versão parcialmente fechada
*/
unsigned int gameId = 0;

char* _init_str(unsigned int _size) {
  char* output = new char[_size];
  for(unsigned int ai = 0;ai < _size;++ai) {
    output[ai] = 0;
  }
  return output;
}

void copy(char* src, char* dst, int len) {
    for (int i = 0; i < len; i++) {
        *dst++ = *src++;
    }
}
class Player {
  private:
    unsigned int _id;
    char _name[11];
  public:
    Player(unsigned int _id, const char* _name) {
      this->_id = _id;
      strcpy(this->_name, _name);
    }

    unsigned int json(char output[]) {
      unsigned size = sprintf_P(output, PSTR("{\"name\":\"%s\"}"), this->_name);
      output[size++] = 0;
      return size;
    }

    unsigned int id() {
      return this->_id;
    }

    char* name() {
      return this->_name;
    }
};

class Game {
  private:
    int _mode = 0;
    int _start_mode = 0;
    int _totalPlayers = 0;
    int _offlinePlayer = 0;
    bool _offline = false;
    unsigned long _latestEntry = 0;
    bool _wait_opponent = true;
    byte _draworder = 0;
    Player* _players[5];
    Player* starter = NULL;
    unsigned int _positions[5];
    unsigned int note = 0;
    unsigned long rnd(unsigned long min) {
      //return random(min, this->_totalPlayers);
      return min + (rand() % (this->_totalPlayers - min));
    }
  public:
    Game() {
      ++gameId;
    }
    void start() {
      this->_mode = 2;
    }
    unsigned int numberOfPlayers() {
      return this->_totalPlayers;
    }
    void resetWait() {
      if (this->_mode == 0) {
        this->_mode = 1;
      }
    }

    Player* getStarter() {
      return this->starter;
    }

    Player* nextPlayer(Player* p) {
      for (int ai = 0;ai < this->_totalPlayers;++ai) {
        if (p->id() == this->_positions[ai]) {
          return this->getPlayer(this->_positions[(1 + ai) % this->_totalPlayers]);
        }
      }
      return NULL;
    }

    Player* addPlayer(const char* name) {
      nomes[this->_totalPlayers] = name;
      this->_start_mode = 0;
      unsigned int id = 1 + this->_totalPlayers;
      Player* output = this->_players[this->_totalPlayers] = new Player(id, name);
      this->_positions[this->_totalPlayers] = id;
      ++this->_totalPlayers;
      return output;
    }
    bool isActive(unsigned int id) {
      return this->_mode != 0 && gameId == id;
    }
    bool running() {
      return this->_mode >= 2;
    }
    void draw(Player* starter) {
      if (!this->starter) {
        //randomSeed(analogRead(A0));
        srand(time(NULL));
        this->starter = starter;

        // Fisher-Yates Shuffle Algorithm
        for (int ai = 1;ai < this->_totalPlayers;++ai) {
          unsigned int temp = this->_positions[ai - 1];
          unsigned int index = this->rnd(ai);
          this->_positions[ai - 1] = this->_positions[index];
          this->_positions[index] = temp;
        }

        this->finish();
      }
    }

    void finish() {
      this->_mode = 3;
      this->_latestEntry = millis();

      lcd.clear();
      lcd.print("Total: ");
      lcd.print(this->_totalPlayers);
      lcd.setCursor(0, 1);
      lcd.print(this->starter->name());
      lcd.print(" sorteou");
      ledGreen.turnOff();
      ledRed.turnOn();
      buzzer.beep(1);
    }

    bool loop() {
      unsigned long diff = millis() - this->_latestEntry;

      return this->_mode == 3 && diff >= 60000;
    }

    Player* getPlayer (unsigned int id) {
      for (unsigned int ai = 0, at = this->_totalPlayers;ai < at;++ai) {
        if (this->_players[ai]->id() == id) {
          return this->_players[ai];
        }
      }
      return NULL;
    }
};

Game* game;

unsigned int responsePlayer(Player* currentPlayer, char output[]);

void aoPressionarBotao() {
  if (game->running()) {
    buzzer.beep(3, 100, 30);
    ledRed.blink(3, 100, 30);
  } else {
    switch (mode) {
      case START_MODE:
        buzzer.beep(3, 100, 30);
        ledRed.blink(3, 100, 30);
        break;
      case END_MODE:
        mode = START_MODE;
        sorteados = 0;
        for (int ai = 0;ai < MAX_SORTEADOS;++ai) {
          numSorteados[ai] = ai;
        }
        break;
      case WAIT_MODE:
        mode = START_MODE;
        buzzer.beep(1);
        lastStarted = millis();
        ledGreen.turnOff();
        ledRed.turnOn();
        lcd.clear();
        lcd.print("De: ");
        lcd.print(nomes[sorteados - 1]);
        lcd.setCursor(0, 1);
        lcd.print("Para: ");
        debug("Sorteio ");
        debug(sorteados);
        debug(" - De: ");
        debug(nomes[sorteados - 1]);
        debug(" -> Para: ");
        if (sorteados == MAX_SORTEADOS) {
          lcd.print(nomes[numSorteados[sorteados - 1]]);
          debugln(nomes[numSorteados[sorteados - 1]]);
        } else {
          unsigned int index = rnd(sorteados);
          int temp = numSorteados[index];
          numSorteados[index] = numSorteados[sorteados - 1];
          numSorteados[sorteados - 1] = temp;
          lcd.print(nomes[temp]);
          debugln(nomes[temp]);
        }
        break;
    }
  }
}

void setup_button() {
  button.onSinglePress(aoPressionarBotao);
}

void loop_amigo_secreto() {
  if (!game->running() && mode == START_MODE && millis() - lastStarted >= TEMPO_AMIGO) {
    mode = WAIT_MODE;
    ledGreen.turnOn();
    ledRed.turnOff();
    lcd.clear();
    if (sorteados == MAX_SORTEADOS) {
      mode = END_MODE;
      buzzer.beep(1);
      lcd.print("Todos sorteados");
      lcd.setCursor(0, 1);
      lcd.print("Reiniciar?");
    } else {
      buzzer.beep(2);
      lcd.print("Passe para");
      lcd.setCursor(0, 1);
      lcd.print(nomes[sorteados]);
    }
    ++sorteados;
  }
}

void setup_game() {
  game = new Game();
}

void loop_game() {
  if (game->loop()) {
    setup_game();

    mode = START_MODE;
    sorteados = MAX_SORTEADOS;
  }
  loop_amigo_secreto();
}
