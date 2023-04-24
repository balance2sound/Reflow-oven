#include "screen.h";
#include <avr/wdt.h>
boolean reset_pasado = false;
void setup() {

  if (bitRead(MCUSR, WDRF)) {
    reset_pasado = true; // a watchdog reset occurred
    bitWrite(MCUSR, WDRF, 0);
  }
  inicializar_io();
  inicializar_pantalla();
  if (reset_pasado == true) {
    cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("SYSTEM", 120, 160, 4);
    tft.drawString("RESET", 120, 190, 4);
    tft.drawString("CRITICAL", 110, 220, 4);
    tft.drawString("FAILURE", 110, 250, 4);
    reset_pasado = false;
    delay(5000);
  }
}

void loop() {
  pantalla(); // función principal
}
