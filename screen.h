//#include <TFT_HX8357_Due.h>
//TFT_HX8357_Due tft = TFT_HX8357_Due();
#include <TFT_HX8357.h> // Hardware-specific library
TFT_HX8357 tft = TFT_HX8357();
#include <avr/wdt.h>
#include "image.h";
#include "profiles.h";
#include "pins.h";
#include "pid.h"

//COLORES
#define BROWN      0x5140 //0x5960
#define SKY_BLUE   0x02B5 //0x0318 //0x039B //0x34BF
#define DARK_RED   0x8000
#define DARK_GREY  0x39C7
#define DARK_GREEN 0x1580
#define DARK_WHITE 0xC560


//VARIABLES
boolean borrar_pantalla, custom_seleccion_pantalla_reflow, sel_perfil, sel_tipo_pcb, sel_temperatura, sel_etapa, sel_tiempo;
boolean refrescar, boton_salir, custom_seleccionado, boton_seleccion_anterior, guardado;
int etapa, grafica_x, grafica_y, tiempo_actual, minutos_restantes, segundos_restantes, tiempo_total, tiempo_restante, n;
int seleccion_menu_reflow, seleccion_menu_editar, seleccion_ajustes, anterior_ajustes;


char *textoetapa;
int numero_pantalla;

int pre_perfiles[sizeof(perfiles)][4][2];

// AJUSTES
boolean pre_ver_usuario, pre_ver_referencia, pre_guardar_registros;
boolean pre_debug_usb, pre_apagar_pantalla, pre_impresora_termica, pre_imprimir_resultados;
boolean ver_usuario = true, ver_referencia = true, guardar_registros = true, debug_usb = true, apagar_pantalla = true, impresora_termica = true, imprimir_resultados = true;

//ENCODER
int boton_accion , counter;
int encoderPinA_value;
byte encoderPinA_prev;
//fin encoder

//BOTON
const unsigned long longPressThreshold = 3000, debounceThreshold = 40, retardo_cambio_estado = 100; // the threshold (in milliseconds) before a long press is detected
unsigned long tiempo_delay, tiempo;
unsigned long buttonTimer = 0, buttonPressDuration = 0, entrar_duracion = 0; // stores the time that the button was pressed (relative to boot time) stores the duration
boolean buttonActive = false, longPressActive = false; // indicates if the button is active/pressed
//FIN BOTON

void encoder() {
  //        BOTONES
  encoderPinA_value = digitalRead(CLK);
  if (encoderPinA_value != encoderPinA_prev) { // check if knob is rotating
    if (digitalRead(DT) != encoderPinA_value) {
      counter++;
    } else {
      counter--;
    }
  }
  encoderPinA_prev = encoderPinA_value;
}
void botones() {
  if (!digitalRead(SW))
  {
    if (buttonActive == false)
    {
      buttonActive = true;
      buttonTimer = millis();
    }
    buttonPressDuration = millis() - buttonTimer;
    if ((buttonPressDuration > longPressThreshold) && (longPressActive == false))
    {
      longPressActive = true;
      boton_accion = 2;
      entrar_duracion = millis();
    }
  }
  // button either hasn't been pressed, or has been released
  else
  {
    // if the button was marked as active, it was recently pressed
    if (buttonActive == true)
    {
      // reset the long press active state
      if (longPressActive == true)
      {
        longPressActive = false;
      }
      // we either need to debounce the press (noise) or register a normal/short press
      else
      {
        // if the button press duration exceeds our bounce threshold, then we register a short press
        if (buttonPressDuration > debounceThreshold && buttonPressDuration < 500)
        {
          boton_accion = 1;
          entrar_duracion = millis();
        }
        else
        {
        }
      }
      buttonActive = false;
    }
  }
  if (entrar_duracion + retardo_cambio_estado < millis() && boton_accion == 1) {
    entrar_duracion = 0;
    boton_accion = 0;
  }
  if (entrar_duracion + retardo_cambio_estado < millis() && boton_accion == 2) {
    entrar_duracion = 0;
    boton_accion = 0;
  }
}
void inicializar_io() {
  pinMode(DT, INPUT);
  pinMode(CLK, INPUT);
  pinMode(SW, INPUT_PULLUP); //PORQUE NO TIENE PULLUP CIRCUITO

  // PID
  pinMode(relog, OUTPUT);
  pinMode(sdo, INPUT);
  pinMode(cs1, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(cs1, HIGH);
  pinMode(cs2, OUTPUT);
  digitalWrite(cs2, HIGH);
  pinMode(cs3, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);

  //PINES MEGA INTERRUPCION 2 3 21  20  19  18
  attachInterrupt(digitalPinToInterrupt(DT), encoder, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(SW), botones, LOW);
  buttonActive = true;
}
void boton(int x, int y, int w, int h, uint16_t color, char *texto, uint16_t colortexto, uint16_t fuente) {

  int margen; //margen boton
  switch (fuente) {
    case 1:
      margen = 1;
      break;
    case 2:
      margen = 2;
      break;
    case 3:
      margen = 2;
      break;
    case 4:
      margen = 4;
      break;
    case 5:
      margen = 4;
      break;
  }

  tft.fillRoundRect(x, y, w, h, h * 0.1, color);
  tft.setTextColor(colortexto); // Text with background
  tft.setTextDatum(MC_DATUM); //divide x e y entre 2
  tft.drawString(texto, x + (w / 2), (y + (h / 2)) + margen, fuente);
}
void cuadro_dialogo(int x, int y, int w, int h, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, h * 0.1, color);
  tft.drawRoundRect(x, y, w, h, h * 0.1, TFT_BLACK);
  tft.drawRoundRect(x + 1, y + 1, w - 1, h - 1, h * 0.1, TFT_BLACK);
  tft.drawRoundRect(x + 2, y + 2, w - 2, h - 2, h * 0.1, TFT_BLACK);
  tft.drawRoundRect(x + 3, y + 3, w - 3, h - 3, h * 0.1, TFT_BLACK);
  tft.drawRoundRect(x + 4, y + 4, w - 4, h - 4, h * 0.1, TFT_BLACK);

}
void numero(int x, int y, int32_t numero, uint16_t color, uint16_t fuente) {
  int numero_anterior;
  if (numero = !numero_anterior) {
    tft.drawNumber(numero, x, y, fuente);
    numero_anterior = numero;
  }
}
int calcular_tiempo_total(int numeroperfil) {
  int tiempocalculado;
  tiempocalculado = perfiles[numeroperfil][0][1] + perfiles[numeroperfil][1][1] + perfiles[numeroperfil][2][1];
  return tiempocalculado;
}

// --- PANTALLAS DE REFLOW ---------------------------------------------- PANTALLAS DE REFLOW -------------------------------------- PANTALLAS DE REFLOW ------------

void pantalla_ajustes() {

  if (borrar_pantalla == true) {
    tft.fillScreen(TFT_WHITE);
    tiempo = millis();
    seleccion_ajustes = 0;
    borrar_pantalla = false;
    counter = 0;
    pre_ver_usuario = ver_usuario;
    pre_ver_referencia = ver_referencia;
    pre_guardar_registros = guardar_registros;
    pre_debug_usb = debug_usb;
    pre_apagar_pantalla = apagar_pantalla;
    pre_impresora_termica = impresora_termica;
    pre_imprimir_resultados = imprimir_resultados;
    boton_seleccion_anterior = false;
    guardado = false;
    anterior_ajustes = -1;
  }

  if (counter < 0) {
    counter = 7;
  }
  if (counter > 7) {
    counter = 0;
  }

  seleccion_ajustes = counter;
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLUE);
  tft.drawString(numero_serie, 120, 472, 2);
  tft.drawString(version_programa, 200, 472, 2);
  switch (seleccion_ajustes) {
    case 0:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("SEL & CLICK to CHANGE", 0, 390, 4);
        tft.drawString("LONG press to SAVE & EXIT", 0, 420, 4);
        tft.drawString("CLICK to CANCEL & EXIT", 0, 450, 4);
      }
      if (boton_accion == 1) {
        numero_pantalla = 1;
        borrar_pantalla = true;
      }
      if (boton_accion == 2) {
        ver_usuario = pre_ver_usuario;
        ver_referencia = pre_ver_referencia;
        guardar_registros = pre_guardar_registros;
        debug_usb = pre_debug_usb;
        apagar_pantalla = pre_apagar_pantalla;
        impresora_termica = pre_impresora_termica;
        imprimir_resultados = pre_imprimir_resultados;
        numero_pantalla = 1;
        borrar_pantalla = true;
      }

      break;
    case 1:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Show user?", 60, 20, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Show user in", 0, 420, 4);
        tft.drawString("home page", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_ver_usuario = !pre_ver_usuario;
        tiempo = millis();
      }
      break;
    case 2:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);

      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      tft.fillRect(0, 220, 320, 5, TFT_BLACK);
      tft.fillRect(0, 370, 320, 95, TFT_BLACK);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Show year / reference", 0, 420, 4);
        tft.drawString("in home page", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_ver_referencia = !pre_ver_referencia;
        tiempo = millis();
      }
      break;
    case 3:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Save records?", 60, 80, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);

      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Save records in SD", 0, 420, 4);
        tft.drawString("if available", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_guardar_registros = !pre_guardar_registros;
        tiempo = millis();
      }
      break;
    case 4:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);

      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Save records & debug", 0, 420, 4);
        tft.drawString("and send to USB", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_debug_usb = !pre_debug_usb;
        tiempo = millis();
      }
      break;
    case 5:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);

      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Turn off screen", 0, 420, 4);
        tft.drawString("in standby mode", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_apagar_pantalla = !pre_apagar_pantalla;
        tiempo = millis();
      }
      break;
    case 6:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);

      tft.drawString("Print results?", 60, 200, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Do you want to use", 0, 420, 4);
        tft.drawString("a thermal printer?", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_impresora_termica = !pre_impresora_termica;
        tiempo = millis();
      }
      break;
    case 7:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Print results?", 60, 200, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 220, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Print results in", 0, 420, 4);
        tft.drawString("thermal printer?", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_imprimir_resultados = !pre_imprimir_resultados;
        tiempo = millis();
      }
      break;
  }
  tft.fillRect(250, 0, 70, 220, TFT_WHITE);
  tft.fillRect(250, 225, 70, 145, TFT_WHITE);
  if (pre_ver_usuario == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 20, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 20, 4);
  }
  if (pre_ver_referencia == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 50, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 50, 4);
  }

  if (pre_guardar_registros == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 80, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 80, 4);
  }
  if (pre_debug_usb == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 110, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 110, 4);
  }
  if (pre_apagar_pantalla == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 140, 4);; //
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 140, 4);; //
  }
  if (pre_impresora_termica == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 170, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 170, 4);
  }
  if (pre_imprimir_resultados == true) {
    tft.setTextColor(DARK_GREEN);
    tft.drawString("YES", 280, 200, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 200, 4);
  }
  anterior_ajustes = seleccion_ajustes;


}
void pantalla_principal() {
  if (borrar_pantalla == true) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLUE); // Text with background
    tft.setTextDatum(MC_DATUM);
    tft.drawString(user, (320 / 2), 420 - 8, 2);
    tft.setTextColor(TFT_BLACK);
    tft.drawString(description, (320 / 2), 450 - 8, 1);
    counter = 1;
    borrar_pantalla = false;
  }

  if (counter < 1) {
    counter = 4;
  }
  if (counter > 4) {
    counter = 1;
  }
  switch (counter) {
    case 1:
      boton(60, 30, 200, 80, DARK_GREEN, "REFLOW", TFT_WHITE, 4);
      boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
      boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);
      if (boton_accion == 2) {
        Serial.print("ajustes");
        numero_pantalla = 5;
        borrar_pantalla = true;
        boton_accion = 0;
      }

      break;
    case 2:
      boton(60, 30, 200, 80, TFT_BLACK, "REFLOW", TFT_WHITE, 4);
      boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
      boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);

      if (boton_accion == 1) {
        numero_pantalla = 2;
        borrar_pantalla = true;
        boton_accion = 0;
      }
      break;

    case 3:
      boton(60, 30, 200, 80, DARK_GREEN, "REFLOW", TFT_WHITE, 4);
      boton(60, 140, 200, 80, TFT_BLACK, "PROFILES", TFT_WHITE, 4);
      boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);

      if (boton_accion == 1) {
        numero_pantalla = 3;
        borrar_pantalla = true;
        boton_accion = 0;

      }
      break;
    case 4:
      boton(60, 30, 200, 80, DARK_GREEN, "REFLOW", TFT_WHITE, 4);
      boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
      boton(60, 250, 200, 80, TFT_BLACK, "LOG", TFT_WHITE, 4);

      if (boton_accion == 1) {

        numero_pantalla = 5;
        borrar_pantalla = true;
        boton_accion = 0;

      }
      break;
  }
}
void pantalla_reflow() {

  //Cargar plantilla
  if (borrar_pantalla == true) {

    tiempo_restante = 0;
    tft.fillScreen(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.fillRect(0, 245, 320, 5, TFT_BLACK);
    tft.fillRect(0, 350, 320, 5, TFT_BLACK);
    tft.fillRect(157, 250, 5, 100, TFT_BLACK);
    tft.drawBitmap(10, 20, imagen_reflow, 300, 225, TFT_BLACK);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Act. Temp", 25 + 50, 255 + 17, 4);
    tft.drawString("Time Left", 190 + 50, 255 + 17, 4);

    sel_tipo_pcb = false;
    sel_perfil = false;
    refrescar = false;
    counter = 0;
    boton_seleccion_anterior = false;
    temperatura_actual = int(leer_temperatura(1)); //LEER TEMPERATURA INTERIOR CON FUNCION READ TEMP
    custom_seleccionado = false;
    seleccion_menu_reflow = 0;
    borrar_pantalla = false;
    tiempo_total = 0;
  }
  //FIN Cargar plantilla
  switch (etapa) {
    case 1:
      textoetapa = "Stopped";
      break;
    case 2:
      textoetapa = "Ramp";
      break;
    case 3:
      textoetapa = "Pre-Heat";
      break;
    case 4:
      textoetapa = "Reflow";
      break;
    case 5:
      textoetapa = "Cooldown";
      break;
    case 6:
      textoetapa = "Finished";
      break;
  }

  //GRAFICO
  /* ESPACIO: X: 240 , Y: 144
    MARGEN IZQUIERDO ORD ORIGEN: 38PX X
    MARGEN SUPERIOR ORD ORIGEN: 200PX Y
  */
  // ------ trabajo iniciado??? ----------------------------------------------- trabajo iniciado??? --------------------------

  //trabajo NO INICIADO
  if (trabajo_iniciado == false) {
    etapa = 1;
    minutos_restantes = 99;
    segundos_restantes = 99;
    //inicio seleccion
    if (sel_perfil == false && sel_tipo_pcb == false) {
      if (counter < 0) {
        counter = 4;
      }
      if (counter > 4) {
        counter = 0;
      }
      seleccion_menu_reflow = counter;
    }
    switch (seleccion_menu_reflow) {
      case 0:
        boton(107, 430, 105, 40, DARK_GREEN, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        break;

      case 1: // perfil seleccionado
        boton(107, 430, 105, 40, DARK_GREEN, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_perfil = true;
          counter = 0;
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
          sel_perfil = false;
          boton_seleccion_anterior = false;
          counter = 1;
          boton_accion = 0;
          tiempo_delay = 0;
        }


        if (sel_tipo_pcb == false && sel_perfil == true) {
          if (counter < 0) {
            counter = numero_perfiles;
          }
          if (counter > numero_perfiles) {
            counter = 0;
          }
          id_perfil = counter;
          tft.fillRect(0, 360, 320, 30, TFT_WHITE);
          tft.setTextColor(TFT_RED);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        }
        break;
      case 2: //tipo_pcb
        boton(107, 430, 105, 40, DARK_GREEN, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.setTextColor(TFT_BLACK); //SELECCIONADO
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB

        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_tipo_pcb = true;
          counter = 0;
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
          sel_tipo_pcb = false;
          boton_seleccion_anterior = false;
          counter = 2;
          boton_accion = 0;
          tiempo_delay = 0;
        }


        if (sel_tipo_pcb == true && sel_perfil == false) {
          if (counter < 0) {
            counter = numero_tipo_pcb;
          }
          if (counter > numero_tipo_pcb) {
            counter = 0;
          }
          id_tipo_pcb = counter;
          tft.fillRect(0, 390, 320, 30, TFT_WHITE);
          tft.setTextColor(TFT_RED);
          tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        }
        break;
      case 3: //boton seleccionado
        boton(107, 430, 105, 40, TFT_BLACK, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        if (boton_accion == 1 && trabajo_iniciado == false) {
          trabajo_iniciado = true;
          refrescar = true;
          tiempo_total=calcular_tiempo_total(id_perfil);
          iniciar_timer_pid();
          boton(107, 430, 105, 40, TFT_RED, "ABORT", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
          //pid(id_perfil);
          wdt_enable(WDTO_2S);

        }
        break;
      case 4: //salir seleccionado
        boton(107, 430, 105, 40, DARK_GREEN, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLACK, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        if (boton_accion == 1 && trabajo_iniciado == false) {
          borrar_pantalla = true;
          numero_pantalla = 1;
        }
        break;
    }

    //fin seleccion
  }
  if (trabajo_iniciado == false) {
    tiempo_total=calcular_tiempo_total(id_perfil);
    tft.setTextColor(TFT_BLACK);
    tiempo_restante = tiempo_total - tiempo_reflow;
    minutos_restantes = tiempo_restante / 60;
    segundos_restantes = tiempo_restante % 60;
    tft.fillRect(165, 287, 215, 55, TFT_WHITE);
    tft.drawNumber(minutos_restantes, 200, 315, 7);
    tft.drawString(":", 180 + 60, 315, 7);
    tft.drawNumber(segundos_restantes, 280, 315, 7);
  }
  if (refrescar == true);
  {
    tiempo_restante = tiempo_total - tiempo_reflow;
    minutos_restantes = tiempo_restante / 60;
    segundos_restantes = tiempo_restante % 60;
    int ancho_barra = (tiempo_total/240);
    tft.setTextColor(BROWN);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(textoetapa, 160, 25, 4);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED);
    tft.fillRect(24, 287, 120, 51, TFT_WHITE);
    tft.drawNumber(temperatura_actual, 80, 315, 7);
    tft.fillRect(165, 287, 215, 55, TFT_WHITE);
    if (tiempo_restante>0) {
      tft.setTextColor(TFT_RED);
      tft.drawNumber(minutos_restantes, 200, 315, 7);
      tft.drawString(":", 180 + 60, 315, 7);
      tft.drawNumber(segundos_restantes, 280, 315, 7);
      grafica_x = 38 + map(tiempo_reflow, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
      grafica_y = 200 - map(temperatura_actual, 0, 300, 0, 144);
      tft.fillRect(grafica_x, grafica_y, ancho_barra, 3, TFT_YELLOW);
    }
    if (tiempo_restante == 0 && calidad_pasada == true && trabajo_iniciado == true) {
      tft.setTextDatum(MC_DATUM);
      tft.fillRect(165, 250, 215, 70, TFT_WHITE);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Quality Stat", 240, 272, 4);
      tft.setTextColor(DARK_GREEN);
      tft.drawString("PASS", 240, 315, 4);
      refrescar = false;
      finalizado = true;
      wdt_disable();
    }
    if (tiempo_restante == 0 && calidad_pasada == false && trabajo_iniciado == true) {
      tft.setTextDatum(MC_DATUM);
      tft.fillRect(165, 250, 215, 70, TFT_WHITE);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Quality Stat", 270, 272, 4);
      tft.setTextColor(TFT_RED);
      tft.drawString("FAIL", 260, 315, 4);
      refrescar = false;
      finalizado = true;
      wdt_disable();
    }
  }
  if (boton_accion == 2 && trabajo_iniciado == true) {
    boton(107, 430, 105, 40, DARK_GREEN, "Start", TFT_WHITE, 4);
    boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
    abortado = true;
    trabajo_iniciado = false;
    wdt_disable();
    cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("ABORTED", 160, 210, 4);
    delay(5000);
    borrar_pantalla = true;
  }

}
void pantalla_editar() {

  int tiempo;
  //Cargar plantilla
  if (borrar_pantalla == true) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Loading data...", 140, 220, 4);
    for (int i = 0; i < sizeof(perfiles); i++) {
      pre_perfiles[i][0][0] = perfiles[i][0][0]; //temperatura 1
      pre_perfiles[i][1][0] = perfiles[i][1][0]; //temperatura 2
      pre_perfiles[i][2][0] = perfiles[i][2][0]; //temperatura 3
      pre_perfiles[i][3][0] = perfiles[i][3][0]; ///temperatura 4
      pre_perfiles[i][0][1] = perfiles[i][0][1]; //tiempo 1
      pre_perfiles[i][1][1] = perfiles[i][1][1]; //tiempo 2
      pre_perfiles[i][2][1] = perfiles[i][2][1]; //tiempo
    }
    tft.fillRect(0, 200, 320, 80, TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.fillRect(0, 245, 320, 5, TFT_BLACK);
    tft.fillRect(0, 350, 320, 5, TFT_BLACK);
    tft.setTextColor(BROWN);
    tft.drawString("Temp", 55, 272, 4);
    tft.drawString("Seconds", 260, 272, 4);
    tft.drawString("STEP", 160, 272, 4);
    tft.setTextColor(BROWN);
    tft.drawBitmap(10, 20, imagen_reflow, 300, 225, TFT_BLACK);
    tft.setTextColor(BROWN);
    tft.drawString("Ramp", 65, 120, 2); //etapa 1
    tft.drawString("Pre-Heat", 150, 110, 2); //etapa 2
    tft.drawString("Reflow", 224, 140, 2); //etapa 3
    tft.drawString("Cooldown", 280, 100, 2); //etapa 4
    sel_temperatura = false;
    sel_etapa = false;
    sel_tiempo = false;
    sel_perfil = false;
    boton_seleccion_anterior = false;
    guardado = false;
    counter = 0;
    id_perfil = 0;
    etapa = 0;
    boton_accion = 0;
    seleccion_menu_editar = 0;
    borrar_pantalla = false;
  }

  switch (etapa) {
    case 0:
      textoetapa = "Ramp";
      break;
    case 1:
      textoetapa = "Pre-Heat";
      break;
    case 2:
      textoetapa = "Reflow";
      break;
    case 3:
      textoetapa = "Cooldown";
      break;
  }
  //Fin plantilla
  tft.setTextColor(TFT_BLUE);
  tft.fillRect(40, 75, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][0][0], 65, 90, 4); //temp etapa 1
  tft.fillRect(120, 65, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][1][0], 145, 80, 4); //temp etapa 2
  tft.fillRect(194, 35, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][2][0], 219, 50, 4); //temp etapa 3
  tft.fillRect(250, 65, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][3][0], 275, 80, 4); //temp etapa 4
  //TIEMPO

  tft.fillRect(40, 215, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][0][1], 65, 230, 4); //tiempo etapa 1
  tft.fillRect(110 + 15, 160, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][1][1], 150, 180, 4); //tiempo etapa 2
  tft.fillRect(195, 160, 50, 30, TFT_WHITE);
  tft.drawNumber(pre_perfiles[id_perfil][2][1], 220, 180, 4); //tiempo etapa 3

  if (etapa == 3) {
    tft.setTextColor(TFT_BLACK);
    tft.drawString("(None)", 260, 339, 2);
  } else {
    tft.fillRect(230, 328, 60, 20, TFT_WHITE);
  }

  //Escribir datos
  if (guardado == false) {
    if (sel_perfil == false && sel_temperatura == false && sel_tiempo == false && sel_etapa == false) {
      if (counter < 0) {
        counter = 6;
      }
      if (counter > 6) {
        counter = 0;
      }
      seleccion_menu_editar = counter;
    }
    switch (seleccion_menu_editar) {
      case 0:
        tft.setTextColor(DARK_GREEN);

        tft.setTextColor(TFT_BLUE);
        tft.drawString(textoetapa, 160, 315, 4);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);

        tft.fillRect(225, 290, 90, 40, TFT_WHITE);
        tft.setTextColor(TFT_BLUE);
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.drawNumber(tiempo, 270, 315, 6);
        temperatura_actual = pre_perfiles[id_perfil][etapa][0];
        tft.fillRect(8, 280, 90, 60, TFT_WHITE);
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);

        break;

      case 1: //seleccion perfil
        temperatura_actual = pre_perfiles[id_perfil][etapa][0];
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.fillRect(225, 290, 90, 40, TFT_WHITE);
        tft.fillRect(8, 280, 90, 60, TFT_WHITE);
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);
        tft.drawNumber(tiempo, 270, 315, 6);
        tft.drawString(textoetapa, 160, 315, 4);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);

        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          counter = 0;
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
          sel_perfil = true;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
          sel_perfil = false;
          boton_seleccion_anterior = false;
          counter = 1;
          boton_accion = 0;
          tiempo_delay = 0;
        }
        if (sel_perfil == true && sel_temperatura == false && sel_tiempo == false && sel_etapa == false) {
          if (counter < 0 ) {
            counter = numero_perfiles;
          }
          if (counter > numero_perfiles) {
            counter = 0;
          }
          id_perfil = counter;
          tft.fillRect(0, 360, 320, 30, TFT_WHITE);
          tft.setTextColor(TFT_RED);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          pre_perfiles[id_perfil][0][0] = perfiles[id_perfil][0][0]; //temperatura 1
          pre_perfiles[id_perfil][1][0] = perfiles[id_perfil][1][0]; //temperatura 2
          pre_perfiles[id_perfil][2][0] = perfiles[id_perfil][2][0]; //temperatura 3
          pre_perfiles[id_perfil][3][0] = perfiles[id_perfil][3][0]; ///temperatura 4
          pre_perfiles[id_perfil][0][1] = perfiles[id_perfil][0][1]; //tiempo 1
          pre_perfiles[id_perfil][1][1] = perfiles[id_perfil][1][1]; //tiempo 2
          pre_perfiles[id_perfil][2][1] = perfiles[id_perfil][2][1]; //tiempo 3
        }

        break;

      case 2: //seleccion temperatura
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLUE);
        tft.drawString(textoetapa, 160, 315, 4);
        tft.drawNumber(tiempo, 270, 315, 6);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.setTextColor(TFT_BLACK);
        tft.drawNumber(temperatura_actual, 55, 315, 6);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        //{perfil{etapa{temperatura, segundos}}}

        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_temperatura = true;
          counter = pre_perfiles[id_perfil][etapa][0];
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
        }
        if (sel_perfil == false && sel_temperatura == true && sel_tiempo == false && sel_etapa == false) {
          if (counter < 0) {
            counter = 320;
          }
          if (counter > 320) {
            counter = 0;
          }

          pre_perfiles[id_perfil][etapa][0] = counter;
          temperatura_actual = counter;
          tft.fillRect(8, 280, 90, 60, TFT_WHITE);
          tft.setTextColor(TFT_RED);
          tft.drawNumber(temperatura_actual, 55, 315, 6);
          if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
            sel_temperatura = false;
            boton_seleccion_anterior = false;
            counter = 2;
            boton_accion = 0;
            tiempo_delay = 0;
          }
        }



        break;

      case 3: //seleccion etapa
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);
        tft.drawNumber(tiempo, 270, 315, 6);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.setTextColor(TFT_BLACK);
        tft.drawString(textoetapa, 160, 315, 4);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_etapa = true;
          counter = 0;
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
        }
        if (sel_perfil == false && sel_temperatura == false && sel_tiempo == false && sel_etapa == true) {
          if (counter < 0) {
            counter = 3;
          }
          if (counter > 3) {
            counter = 0;
          }
          etapa = counter;

          tft.fillRect(225, 290, 90, 40, TFT_WHITE);
          tft.setTextColor(TFT_BLUE);
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.drawNumber(tiempo, 270, 315, 6);
          temperatura_actual = pre_perfiles[id_perfil][etapa][0];
          tft.fillRect(8, 280, 90, 60, TFT_WHITE);
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);

          tft.fillRect(100, 300, 125, 35, TFT_WHITE);
          tft.setTextColor(TFT_RED);
          tft.drawString(textoetapa, 160, 315, 4);
          if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
            sel_etapa = false;
            boton_seleccion_anterior = false;
            counter = 3;
            boton_accion = 0;
            tiempo_delay = 0;
          }
        }
        break;
      case 4: //seleccion tiempo
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);

        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(textoetapa, 160, 315, 4);
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLACK);
        tft.drawNumber(tiempo, 270, 315, 6);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);

        if (etapa != 3) {
          if (boton_accion == 1 && boton_seleccion_anterior == false) {
            counter = pre_perfiles[id_perfil][etapa][1];
            sel_tiempo = true;
            boton_seleccion_anterior = true;
            tiempo_delay = millis();
            boton_accion = 0;
          }

          if (sel_perfil == false && sel_temperatura == false && sel_tiempo == true && sel_etapa == false) {
            if (counter < 1) {
              counter = 700;
            }
            if (counter > 700) {
              counter = 1;
            }
            pre_perfiles[id_perfil][etapa][1] = counter;
            tiempo = counter;
            tft.fillRect(225, 290, 90, 40, TFT_WHITE);
            tft.setTextColor(TFT_RED);
            tft.drawNumber(tiempo, 270, 315, 6);
            if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
              sel_tiempo = false;
              boton_seleccion_anterior = false;
              counter = 4;
              boton_accion = 0;
              tiempo_delay = 0;
            }
          }
        }
        break;
      case 5: //seleccion guardar
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);
        tft.drawNumber(tiempo, 270, 315, 6);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(textoetapa, 160, 315, 4);

        boton(107, 430, 105, 40, TFT_BLACK, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);

        if (boton_accion == 1 && guardado == false) {
          cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
          tft.setTextColor(TFT_WHITE);
          tft.drawString("Saving...", 180, 210, 4);
          for (int i = 0; i < sizeof(perfiles); i++) {
            if ( perfiles[i][0][0] != pre_perfiles[i][0][0]) {
              perfiles[i][0][0] = pre_perfiles[i][0][0];
            }
            if (perfiles[i][1][0] != pre_perfiles[i][1][0]) {
              perfiles[i][1][0] = pre_perfiles[i][1][0];
            }
            if (perfiles[i][2][0] != pre_perfiles[i][2][0]) {
              perfiles[i][2][0] = pre_perfiles[i][2][0];
            }
            if (perfiles[i][3][0] != pre_perfiles[i][3][0]) {
              perfiles[i][3][0] = pre_perfiles[i][3][0];
            }
            if (perfiles[i][0][1] != pre_perfiles[i][0][1]) {
              perfiles[i][0][1] = pre_perfiles[i][0][1];
            }
            if (perfiles[i][1][1] != pre_perfiles[i][1][1]) {
              perfiles[i][1][1] = pre_perfiles[i][1][1];
            }
            if (perfiles[i][2][1] != pre_perfiles[i][2][1]) {
              perfiles[i][2][1] = pre_perfiles[i][2][1];
            }
          }
          cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
          tft.setTextColor(TFT_WHITE);
          tft.drawString("Saved", 160, 210, 4);
          delay(2000);
          borrar_pantalla = true;
          numero_pantalla = 1;

        }

        break;
      case 6://seleccion cerrar
        tiempo = pre_perfiles[id_perfil][etapa][1];
        tft.setTextColor(TFT_BLUE);
        tft.drawNumber(temperatura_actual, 55, 315, 6);
        tft.drawNumber(tiempo, 270, 315, 6);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(textoetapa, 160, 315, 4);
        boton(107, 430, 105, 40, DARK_GREEN, "Save", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLACK, "Back", TFT_WHITE, 2);
        if (boton_accion == 1 && guardado == false) {
          borrar_pantalla = true;
          numero_pantalla = 1;
        }
        break;

    }
  }

}
void pantalla_log(){
  
}
void inicializar_pantalla() {

  borrar_pantalla = true;
  tft.init();
  tft.setRotation(0);
  tft.invertDisplay(1);
  counter = 0;
  numero_pantalla = 1;
  tft.fillScreen(TFT_WHITE);
}
void pantalla() {
  //Si interrupcion no usar funcion botones
  botones();


  switch (numero_pantalla) {
    case 1: //pantalla principal
      pantalla_principal();
      break;
    case 2: // pantalla reflow
      pantalla_reflow();
      break;
    case 3: //pantalla profiles (editar)
      pantalla_editar();
      break;
    case 4: //pantalla custom

      break;
    case 5: //pantalla ajustes
      pantalla_ajustes();
      break;
  }
}
