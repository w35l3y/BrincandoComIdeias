WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


/**
 * Função para verificar se a data atual é fim de semana
 * 
 * Retorna true se for 0 (domingo) ou 6 (sábado)
 */
#ifndef DEBUG
bool isWeekend() {
  return timeClient.getDay() % 6 == 0;
}
#else
bool isWeekend () {
  return false;
}
#endif

/**
 * Função para verificar se está na hora esperada para ligar a cafeteira
 * 
 * Retorna true se o horário atual estiver entre o horário esperado e 5 minutos após o horário esperado
 * Ou seja, após os 5 minutos do horário esperado, a cafeteira só religa no próximo dia útil
 */
bool isShowTime() { // se resetar antes do limite, ainda está valendo
  uint currentHour = timeClient.getHours();
  uint currentMinute = timeClient.getMinutes();
  
  return (currentHour == expectedHour && expectedMinute <= currentMinute && currentMinute < 5 + expectedMinute) || (expectedMinute > 55 && currentHour == ((1 + expectedHour) % 24) && currentMinute < expectedMinute - 55);
}
