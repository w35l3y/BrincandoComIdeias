/*
 * Esta biblioteca foi inicialmente desenvolvida para controlar uma pista de corrida de leds
 * Percebi que algumas lógicas poderiam ser mantidas
 * Ela deverá sofrer ainda muitas modificações antes de chegar numa versão parcialmente fechada
 * 
 * Fluxo do Sorteio WEB
 * 1. A pessoa acessa, informa o nome, confirma e aguarda até que todos os demais entrem e confirmem.
 * 2. Depois que todos entrarem, qualquer um pode clicar em "Próximo"
 *    Isso fará com que o sorteio seja realizado.
 *    Todos os demais que clicarem, receberão o nome da pessoa sorteada.
 *    Se alguém tentar entrar após o início do sorteio, receberá a informação que o sorteio já foi realizado.
 *    Passado 1 minuto do sorteio, poderá ser feito um novo sorteio com outras pessoas ou as mesmas.
*/
#include "Game.h"

#define AP_SSID "Amigo Secreto"

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define DNS_PORT 53

ESP8266WebServer server(80);
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

void setup_apwifi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID);

  dnsServer.start(DNS_PORT, "*", apIP);

  IPAddress myIP = WiFi.softAPIP();
  debug("\nAP IP address: ");
  debugln(myIP);

  MDNS.begin("ESP8266");
  MDNS.addService("http", "tcp", 80);
}

const char* MANIFEST_FILE PROGMEM = R"(
{
  "name": "Amigo Secreto",
  "short_name": "Amigo Secreto",
  "start_url": "/",
  "display": "standalone",
  "background_color": "#fff",
  "description": "Amigo Secreto",
  "icons": [],
  "related_applications": [{
    "platform": "web"
  }]
}
)";

const char* INDEX_PAGE PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="manifest" href="/manifest.webmanifest">
<link rel="shortcut icon" type="image/png" sizes="16x16" href="data:image/png;base64,
iVBORw0KGgoAAAANSUhEUgAAABAAAAAQBAMAAADt3eJSAAAAMFBMVEU0OkArMjhobHEoPUPFEBIu
O0L+AAC2FBZ2JyuNICOfGx7xAwTjCAlCNTvVDA1aLzQ3COjMAAAAVUlEQVQI12NgwAaCDSA0888G
CItjn0szWGBJTVoGSCjWs8TleQCQYV95evdxkFT8Kpe0PLDi5WfKd4LUsN5zS1sKFolt8bwAZrCa
GqNYJAgFDEpQAAAzmxafI4vZWwAAAABJRU5ErkJggg==" />
<title>Amigo Secreto</title>
<style>
html, body {
  padding: 0;
  margin: 0;
}

html, body, button {
  font-family: sans-serif;  
}

body {
  background-color: darkgrey;
  margin: 10px;
}

#commands-container {
  display: flex;
  position: absolute;
  bottom: 0;
  right: 0;
  margin: 20px;
}

#play-button {
  height: 80px;
  line-height: 80px;  
  width: 200px;  
  font-size: 2em;
  font-weight: bold;
  border-radius: 30%;
  background-color: #4CAF50;
  color: white;
  text-align: center;
  cursor: pointer;
}

#game, #game_wait {
  display: none;
}

#config-container {
  display: flex;
  flex-direction: column;
}

#config-container > div {
  display: flex;
  flex-direction: column;
}
</style>
<script>
const messages = {
  GAME_STARTED_ALREADY: "Sorteio já iniciou",
  LIMIT_PLAYERS_REACHED: "Limite de participantes alcançado",
  NETWORK_ERROR: "Falha na rede"
}

function $(id) { return document.getElementById(id) }
function update_content(id, content) { $(id).innerHTML = content }

function on_submit (e) {
  e.preventDefault()
  e.stopPropagation()
  
  game.submit()
}

function Game () {
  this.lastError = null
  this.mode = 0
  //$("name").value="Player"

  this.update = ({gameId, playerId, player}) => {
    this.gameId = gameId
    this.playerId = playerId

    if (player) {
      update_content("game", "<div>A pessoa sorteada foi <b>"+player.name+"</b></div>")

      $("game").style.display = "block"
      $("game_wait").style.display = "none"
    } else {
      $("game").style.display = "none"
      $("game_wait").style.display = "block"
    }
  }

  this.start = (obj) => {
    $("config").style.display = "none"

    this.mode = 1
    this.lastError = null
    this.update(obj)
  }

  this.send = obj => {
    return new Promise((resolve, reject) => {
      let req = new XMLHttpRequest()
      req.open('POST', '/', true)
      req.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
      req.onerror = () => reject({status:req.status,message: "NETWORK_ERROR"})
      req.onload = () => {
        if (req.status === 200) resolve(JSON.parse(req.response))
        else reject({...JSON.parse(req.response), status:req.status})
      }
      ["name"].forEach(id => {
        obj[id] = $(id).value
      })
      req.send(Object.entries(obj).map(([key, value]) => key+"="+encodeURIComponent(value)).join("&"))
    }).catch(({message}) => {
      //if (this.lastError !== message) {
        this.lastError = message
        alert(messages[message] || message)
      //}
      throw new Error(message)
    })
  }
  this.submit = () => {
    if (this.mode === 0) {
      this.send({mode: 0}).then(this.start)
    } else {
      //$("play-button").setAttribute("disabled", "disabled");
      this.send({mode: 1, game_id: this.gameId, player_id: this.playerId}).then(this.update).then(() => {
        //$("play-button").removeAttribute("disabled");
      })
    }
  }
}
</script>
</head>
<body>
<form onsubmit='on_submit(event)'>
  <div id="config">
    <div id="config-container">
      <div><label for="name">Nome</label><input required id="name" type="text" minlength="1" maxlength="10" value="" autofocus /></div>
    </div>
  </div>
  <div id="game_wait">
  Aguarde até que todos confirmem a entrada para clicar em "Próximo"<br />
  Assim que clicar, o sorteio será realizado com todos que tiverem entrado até o momento do clique.
  </div>
  <div id="game">&nbsp;</div>
  <div id="commands">
    <div id="commands-container">
      <button type="submit" id="play-button">Próximo</button>
    </div>
  </div>
</form>
<script>
let game = new Game()
</script>
</body>
</html>
)";

unsigned int responsePlayer(Player* currentPlayer, char output[]) {
  char player[3+1*56] = "null";
  char starter[3+1*56] = "null";
  if (game->running()) {
    game->nextPlayer(currentPlayer)->json(player);
    game->getStarter()->json(starter);
  }
  return sprintf_P(output, PSTR("{\"gameId\":%d,\"playerId\":%d,\"player\":%s,\"starter\":%s}"), gameId, currentPlayer->id(), player, starter);
}

void sendError(int status, char* message) {
  char output[114];
  sprintf_P(output, PSTR("{\"message\":\"%s\"}"), message);
  server.send(status, F("application/json; charset=utf-8"), output);
}

void startGame() {
  if (game->running()) {
    sendError(401, (char*)"GAME_STARTED_ALREADY");
    return;
  }
  unsigned int players = game->numberOfPlayers();
  if (players == 5) {
    sendError(401, (char*)"LIMIT_PLAYERS_REACHED");
    return;
  }
  game->resetWait();
  Player* currentPlayer = game->addPlayer(server.arg("name").c_str());
  char buffer[1024];
  responsePlayer(currentPlayer, buffer);
  server.send(200, F("application/json; charset=utf-8"), buffer);
}

void handleRoot() {
  server.send(200, F("text/html; charset=utf-8"), INDEX_PAGE);
}

void handlePlayer() {
  switch (server.arg("mode").toInt()) {
    case 0:
      startGame();
      break;
    case 1:
      if (!game->isActive(server.arg("game_id").toInt())) {
        startGame();
        return;
      }
      unsigned int playerId = server.arg("player_id").toInt();
      Player* currentPlayer = game->getPlayer(playerId);
      game->draw(currentPlayer);
      char buffer[1024];
      responsePlayer(currentPlayer, buffer);
      server.send(200, F("application/json; charset=utf-8"), buffer);
      break;
  }
}

//void handleFavicon() {
//  server.send(200, F("image/x-icon"), FAVICON_FILE);
//}

void handleManifest() {
  server.send(200, F("application/manifest+json; charset=utf-8"), MANIFEST_FILE);
}

void setup_server() {
  setup_game();
  setup_apwifi();

  server.on("/", HTTP_GET, handleRoot);
  //server.on("/favicon.ico", HTTP_GET, handleFavicon);
  server.on("/manifest.webmanifest", HTTP_GET, handleManifest);

  server.on("/", HTTP_POST, handlePlayer);
  server.begin();
}

void loop_server() {
  loop_game();

  MDNS.update();
  dnsServer.processNextRequest();
  server.handleClient();
}
