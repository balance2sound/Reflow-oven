//#include <TFT_HX8357_Due.h>
//TFT_HX8357_Due tft = TFT_HX8357_Due();
#include <TFT_HX8357.h> // Hardware-specific library
TFT_HX8357 tft = TFT_HX8357();
#include <avr/wdt.h>
#include "image.h";
#include "profiles.h";
#include "pins.h";
#include "pid.h"
#include "coms.h"

//COLORES
#define BROWN      0xAAA0 //0x5960
#define SKY_BLUE   0x0318 //0x0318 
#define DARK_RED   0x8000 //0x8000
#define DARK_GREY  0x9CD3 //0x18E3   DA ERRORES CON PUNTEROS
#define TFT_GREEN  0x0780 //0x0780
#define DARK_WHITE 0xC560 //0xC560
#define TFT_ORANGE 0xFC00

//VARIABLES
boolean borrar_pantalla, custom_seleccion_pantalla_reflow, sel_perfil, sel_tipo_pcb, sel_temperatura, sel_etapa, sel_tiempo, sel_pendiente, refrescar_grafica;
boolean boton_salir, custom_seleccionado, boton_seleccion_anterior, guardado, mostrar_pendiente, actualizar_seleccion;
int etapa, n;
float grafica_x, grafica_x_anterior, grafica_y_anterior, grafica_y, ancho_barra, altura_barra, pid_grafica_x, pid_grafica_y, pid_grafica_x_anterior;
int minutos_restantes, segundos_restantes, tiempo_restante, tiempo_restante_anterior;
int seleccion_menu_reflow, seleccion_menu_editar, seleccion_ajustes, anterior_ajustes;


char *textoetapa;
int numero_pantalla;
int pre_perfiles[sizeof(perfiles)][5][2];


int numero_log, id_archivo, numero_archivo, anterior_numero_log;
// AJUSTES
boolean pre_ver_usuario, pre_ver_referencia, pre_guardar_registros, pre_mostrar_pid;
boolean pre_debug_usb, pre_apagar_pantalla, pre_impresora_termica, pre_imprimir_resultados;
boolean ver_usuario = true, ver_referencia = true, guardar_registros = true, debug_usb = true, apagar_pantalla = true, impresora_termica = true;
boolean mostrar_pid = true, imprimir_resultados = true;

//ENCODER
int boton_accion , counter, counter_anterior;
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
  // ENCODER
  pinMode(DT, INPUT);
  pinMode(CLK, INPUT);
  pinMode(SW, INPUT_PULLUP); //PORQUE NO TIENE PULLUP CIRCUITO
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, LOW);
  // PID
  pinMode(relog, OUTPUT);
  pinMode(sdo, INPUT);
  pinMode(cs1, OUTPUT);
  pinMode(cs2, OUTPUT);
  pinMode(cs3, OUTPUT);

  //IO
  //pinMode(zumbador, OUTPUT);
  digitalWrite(cs1, HIGH);
  pinMode(cs2, OUTPUT);
  digitalWrite(cs2, HIGH);
  pinMode(cs3, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  //PINES MEGA INTERRUPCION 2 3 21  20  19  18
  //attachInterrupt(digitalPinToInterrupt(DT), encoder, CHANGE);

  inicializar_coms();

  //DEBUG
  pinMode(vcc, OUTPUT);
  digitalWrite(vcc, HIGH);
  pinMode(gnd, OUTPUT);
  digitalWrite(gnd, LOW);
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
int calcular_tiempo_total(int numeroperfil) {
  int tiempocalculado;
  tiempocalculado = perfiles[numeroperfil][0][1] + perfiles[numeroperfil][1][1] + perfiles[numeroperfil][2][1] + perfiles[numeroperfil][3][1] + perfiles[numeroperfil][4][1];
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
    pre_mostrar_pid = mostrar_pid;
    boton_seleccion_anterior = false;
    guardado = false;
    anterior_ajustes = -1;
    counter_anterior = -1;
  }

  if (counter < 0) {
    counter = 8;
  }
  if (counter > 8) {
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
        mostrar_pid = pre_mostrar_pid;
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);

      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);
      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
      tft.drawString("Show PID on graph?", 60, 230, 4);

      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
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
    case 8:
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_BLUE);
      tft.drawString("Show PID on graph?", 60, 230, 4);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("Show user?", 60, 20, 4);
      tft.drawString("Show year/reference?", 60, 50, 4);
      tft.drawString("Save records?", 60, 80, 4);
      tft.drawString("Send status via usb?", 60, 110, 4);
      tft.drawString("Turn off screen?", 60, 140, 4);
      tft.drawString("Thermal printer?", 60, 170, 4);
      tft.drawString("Print results?", 60, 200, 4);

      if (seleccion_ajustes != anterior_ajustes) {
        tft.fillRect(0, 250, 320, 5, TFT_BLACK);
        tft.fillRect(0, 370, 320, 95, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("INFO & DESCRIPTION", 20, 390, 4);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("Show PID response on", 0, 420, 4);
        tft.drawString("the reflow graph?", 0, 450, 4);
      }
      if (boton_accion == 1) {
        pre_mostrar_pid = !pre_mostrar_pid;
        tiempo = millis();
      }
      break;
  }

  tft.fillRect(250, 0, 70, 250, TFT_WHITE);


  if (pre_ver_usuario == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 20, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 20, 4);
  }
  if (pre_ver_referencia == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 50, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 50, 4);
  }

  if (pre_guardar_registros == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 80, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 80, 4);
  }
  if (pre_debug_usb == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 110, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 110, 4);
  }
  if (pre_apagar_pantalla == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 140, 4);; //
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 140, 4);; //
  }
  if (pre_impresora_termica == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 170, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 170, 4);
  }
  if (pre_imprimir_resultados == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 200, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 200, 4);
  }
  if (pre_mostrar_pid == true) {
    tft.setTextColor(TFT_GREEN);
    tft.drawString("YES", 280, 230, 4);
  } else {
    tft.setTextColor(TFT_RED);
    tft.drawString("NO", 280, 230, 4);
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
    tft.drawString(description, (320 / 2), 434, 1);
    boton(60, 30, 200, 80, TFT_GREEN, "REFLOW", TFT_WHITE, 4);
    boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
    boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);
    counter = 1;
    if (serial_disponible == true) {
      tft.setTextColor(TFT_GREEN);
      tft.drawString("USB Connected", 260, 460, 2);
    }
    if (sd_disponible == true) {
      tft.setTextColor(TFT_GREEN);
      tft.drawString("SD Card", 35, 460, 2);
    }
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
      if (counter != counter_anterior) {
        boton(60, 30, 200, 80, TFT_GREEN, "REFLOW", TFT_WHITE, 4);
        boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
        boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);
      }
      if (boton_accion == 2) {
        Serial.print("ajustes");
        numero_pantalla = 5;
        borrar_pantalla = true;
        boton_accion = 0;
      }

      break;
    case 2:
      if (counter != counter_anterior) {
        boton(60, 30, 200, 80, TFT_BLACK, "REFLOW", TFT_WHITE, 4);
        boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
        boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);
      }
      if (boton_accion == 1) {
        numero_pantalla = 2;
        borrar_pantalla = true;
        boton_accion = 0;
      }
      break;

    case 3:
      if (counter != counter_anterior) {
        boton(60, 30, 200, 80, TFT_GREEN, "REFLOW", TFT_WHITE, 4);
        boton(60, 140, 200, 80, TFT_BLACK, "PROFILES", TFT_WHITE, 4);
        boton(60, 250, 200, 80, TFT_PURPLE, "LOG", TFT_WHITE, 4);
      }
      if (boton_accion == 1) {
        numero_pantalla = 3;
        borrar_pantalla = true;
        boton_accion = 0;

      }
      break;
    case 4:
      if (counter != counter_anterior) {
        boton(60, 30, 200, 80, TFT_GREEN, "REFLOW", TFT_WHITE, 4);
        boton(60, 140, 200, 80, TFT_ORANGE, "PROFILES", TFT_WHITE, 4);
        boton(60, 250, 200, 80, TFT_BLACK, "LOG", TFT_WHITE, 4);
      }
      if (boton_accion == 1) {

        numero_pantalla = 4;
        borrar_pantalla = true;
        boton_accion = 0;

      }
      break;
  }
  counter_anterior = counter;
}
void calcular_grafica(int ident_perfil) {

  //GRAFICO
  /* ESPACIO: X: 240 , Y: 144
    MARGEN IZQUIERDO ORD ORIGEN: 38PX X
    MARGEN SUPERIOR ORD ORIGEN: 200PX Y
  */

  //sacar eje x de cada etapa. Se calcula el tiempo mapeado total
  //aplicar factor de correccion del total
  float a, b;
  int tiempo_total_no_map = calcular_tiempo_total(ident_perfil);
  int tiempo_ramp = perfiles[ident_perfil][0][1];
  int tiempo_preheat = perfiles[ident_perfil][1][1] + tiempo_ramp;
  int tiempo_ramp_reflow = perfiles[ident_perfil][2][1] + tiempo_preheat;
  int tiempo_reflow = perfiles[ident_perfil][3][1] + tiempo_ramp_reflow;
  int tiempo_cooldown = perfiles[ident_perfil][4][1] + tiempo_reflow;
  int i = 0;
  int etapa = 0;
  float pendiente;
  tft.fillRect(5, 5, 310, 240, TFT_WHITE);


  do {
    switch (etapa) {
      case 0:
        grafica_x_anterior = 0;
        grafica_y_anterior = 0;
        ancho_barra = 244 / tiempo_total_no_map;
        a = 20; //cambiar a promedio
        b = ((perfiles[ident_perfil][0][0] - a) / perfiles[ident_perfil][0][1]); //temperatura entre tiempo  map(tiempo_trabajo, 0, tiempo_total, 0, 240)
        altura_barra = b;
        etapa = 1;
        break;
      case 1: //RAMP
        grafica_y = (b * i) + a;

        if (i > tiempo_ramp) { //editar aqui
          if (perfiles[ident_perfil][0][0] == perfiles[ident_perfil][1][0]) {
            a = perfiles[ident_perfil][1][0];
            b = 0;
            altura_barra = 0;
          } else {
            a = perfiles[ident_perfil][0][0]; //cambiar a promedio
            b = (perfiles[ident_perfil][1][0] - a) / perfiles[ident_perfil][1][1]; //temperatura entre tiempo
            altura_barra = b;
          }
          etapa = 2;
        }
        break;
      case 2: //PREHEAT editar aqui
        //actualizacion variable siguiente paso
        grafica_y = (b * (i - tiempo_ramp)) + a;
        if (i > tiempo_preheat) {
          a = perfiles[ident_perfil][1][0]; //cambiar a promedio
          b = (perfiles[ident_perfil][2][0] - a) / perfiles[ident_perfil][2][1]; //temperatura entre tiempo
          altura_barra = b;
          etapa = 3;
        }
        break;
      case 3: //RAMP_REFLOW
        grafica_y = (b * (i - tiempo_preheat)) + a;

        if (i > tiempo_ramp_reflow) {
          grafica_y = perfiles[ident_perfil][3][0];
          altura_barra = 0;
          etapa = 4;
        }
        break;
      case 4: //REFLOW
        if (i > tiempo_reflow) {
          a = perfiles[ident_perfil][3][0];
          b = (perfiles[ident_perfil][3][0]) / perfiles[ident_perfil][4][1]; //temperatura entre tiempo
          altura_barra = b;
          etapa = 5;
        }
        break;
      case 5: //COOLDOWN
        grafica_y = a - (b * (i - tiempo_reflow));
        break;
    }
    tft.fillRect(38 + map(i, 1, tiempo_total_no_map, 0, 244), 202 - map(grafica_y, 1, 300, 0, 144), ancho_barra + 1, altura_barra + 3, TFT_RED);
    i++;
  } while (i <= tiempo_total_no_map);
}
void pantalla_reflow() { //ARREGLAR: evitar actualizacion de numeros si no hay cambio en el perfil. Reflow grafica sigue sin coincidir

  //Cargar plantilla
  if (borrar_pantalla == true) {
    tiempo_restante_anterior = 0;
    tiempo_restante = 0;
    tiempo_ramp = 0;
    tiempo_preheat = 0;
    tiempo_trabajo = 0;
    tiempo_cooldown = 0;
    ancho_barra = 0;
    grafica_x_anterior = 0;
    grafica_x = 0;
    grafica_y = 0;
    pid_grafica_x_anterior = 0;
    pid_grafica_x = 0;
    pid_grafica_y = 0;
    tft.fillScreen(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.fillRect(0, 245, 320, 5, TFT_BLACK);
    tft.fillRect(0, 350, 320, 5, TFT_BLACK);
    tft.fillRect(157, 250, 5, 100, TFT_BLACK);
    tft.setTextColor(TFT_BLUE);
    tft.drawString("Act. Temp", 25 + 50, 255 + 17, 4);
    tft.drawString("Time Left", 190 + 50, 255 + 17, 4);
    finalizado = false;
    abortado = false;
    trabajo_iniciado = false;
    sel_tipo_pcb = false;
    sel_perfil = false;
    counter = 0;
    boton_seleccion_anterior = false;
    temperatura_actual = leer_temperatura(1); //LEER TEMPERATURA INTERIOR CON FUNCION READ TEMP
    custom_seleccionado = false;
    seleccion_menu_reflow = 0;
    tiempo_total = 0;
    etapa = 1;
    borrar_pantalla = false;
    calidad_pasada = true; //----------------------debug
    wdt_disable();
    Timer1.stop(); // quitar?
  }
  //FIN Cargar plantilla

  // ------ trabajo iniciado??? ----------------------------------------------- trabajo iniciado??? --------------------------

  //trabajo NO INICIADO
  if (trabajo_iniciado == false) {
    etapa = 1;
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
        boton(107, 430, 105, 40, TFT_GREEN, "Start", TFT_WHITE, 4);
        boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
        tft.setTextColor(TFT_BLUE);
        tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
        tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB
        break;

      case 1: // perfil seleccionado
        boton(107, 430, 105, 40, TFT_GREEN, "Start", TFT_WHITE, 4);
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
        boton(107, 430, 105, 40, TFT_GREEN, "Start", TFT_WHITE, 4);
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
        if (trabajo_iniciado == false) {
          boton(107, 430, 105, 40, TFT_BLACK, "Start", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
          tft.setTextColor(TFT_BLUE);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.drawString(texto_tipo_pcb[id_tipo_pcb], 160, 405, 4); //CARAS PCB

          if (boton_accion == 1 && finalizado == false) {
            temperatura_actual = leer_temperatura(1); ///////////////////cambiar void si se leen solo 2
            trabajo_iniciado = true;
            //calcular tiempos y pasarlo a archivo pid.h
            tiempo_total = calcular_tiempo_total(id_perfil);
            tiempo_ramp = perfiles[id_perfil][0][1];
            temperatura_ramp = perfiles[id_perfil][0][0];
            tiempo_preheat = perfiles[id_perfil][1][1];
            temperatura_preheat = perfiles[id_perfil][1][0];
            tiempo_reflow = perfiles[id_perfil][2][1];
            temperatura_reflow = perfiles[id_perfil][2][0];
            tiempo_cooldown = perfiles[id_perfil][3][1];
            boton(107, 430, 105, 40, TFT_RED, "ABORT", TFT_WHITE, 4);
            boton(240, 420, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);
            wdt_enable(WDTO_2S);
            inicializar_pid(); //BORRAR SI PRECISA
            iniciar_timer_pid();
            ancho_barra = 240 / tiempo_total; //240
            //detachInterrupt(digitalPinToInterrupt(DT));
            //pid(id_perfil);
            Serial.println("--SOLDERING STARTED--");
            Serial.println("Stats:");
            Serial.println("Profile: ");
            Serial.print(nombre_perfil[id_perfil]);
            Serial.println("Time: ");
            Serial.print(tiempo_total);
          }
        }
        break;
      case 4: //salir seleccionado
        boton(107, 430, 105, 40, TFT_GREEN, "Start", TFT_WHITE, 4);
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
    //calcular tiempos según selección
    tiempo_total = calcular_tiempo_total(id_perfil);
    tft.setTextColor(TFT_BLACK);
    tiempo_restante = tiempo_total - tiempo_trabajo;
    minutos_restantes = tiempo_restante / 60;
    segundos_restantes = tiempo_restante % 60;
    tft.fillRect(165, 287, 215, 55, TFT_WHITE);
    tft.drawNumber(minutos_restantes, 200, 315, 7);
    tft.drawString(":", 180 + 60, 315, 7);
    tft.drawNumber(segundos_restantes, 280, 315, 7);
    tft.fillRect(24, 287, 120, 51, TFT_WHITE);
    tft.drawNumber(temperatura_actual, 80, 315, 7);

  }
  while (trabajo_iniciado == true) { //------------------------ trabajo iniciado = true ----------------------------
    botones();
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
    counter = 0;
    tft.fillRect(238, 416, 70, 30, TFT_WHITE);
    if (boton_accion == 2) {
      abortado = true;
      trabajo_iniciado = false;
      wdt_disable();
      cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("ABORTED", 160, 210, 4);
      borrar_pantalla = true;
      delay(5000);
    }
    tiempo_restante = tiempo_total - tiempo_trabajo;
    if (tiempo_restante != tiempo_restante_anterior && tiempo_restante != 0) {
      etapa = etapa_pid;
      minutos_restantes = tiempo_restante / 60;
      segundos_restantes = tiempo_restante % 60;
      tft.fillRect(100, 8, 180, 40, TFT_WHITE);
      tft.setTextColor(BROWN);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(textoetapa, 160, 25, 4);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_RED);
      tft.fillRect(24, 287, 120, 51, TFT_WHITE);
      tft.drawNumber(temperatura_actual, 80, 315, 7);
      tft.fillRect(165, 287, 215, 55, TFT_WHITE);
      if (tiempo_restante > 0) {
        tft.setTextColor(TFT_RED);
        tft.drawNumber(minutos_restantes, 200, 315, 7);
        tft.drawString(":", 180 + 60, 315, 7);
        tft.drawNumber(segundos_restantes, 280, 315, 7);
        if (mostrar_pid == true) {
          pid_grafica_x = 38 + map(tiempo_trabajo, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
          pid_grafica_x = pid_grafica_x - (pid_grafica_x - pid_grafica_x_anterior);
          pid_grafica_y = 202 - map(pid(), 1, 255, 0, 144);
          tft.fillRect(38 + pid_grafica_x, pid_grafica_y, ancho_barra, 2, TFT_BLUE);
          pid_grafica_x_anterior = pid_grafica_x + ancho_barra;
        }
        grafica_x = 38 + map(tiempo_trabajo, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
        grafica_x = (grafica_x - grafica_x_anterior);
        grafica_y = 202 - map(temperatura_actual, 1, 300, 0, 144);
        tft.fillRect(38 + grafica_x, grafica_y, ancho_barra, 3, TFT_YELLOW);
        grafica_x_anterior = grafica_x + ancho_barra;

        tiempo_restante_anterior = tiempo_restante;

      }
    }
    wdt_reset();
    if (tiempo_restante <= 0) { //si ha finalizado el tiempo
      if (mostrar_pid == true) {
        pid_grafica_x = 38 + map(tiempo_trabajo, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
        pid_grafica_x = (pid_grafica_x - pid_grafica_x_anterior);
        pid_grafica_y = 202 - map(pid(), 1, 255, 0, 144);
        tft.fillRect(38 + pid_grafica_x, pid_grafica_y, ancho_barra, 2, TFT_BLUE);
        pid_grafica_x_anterior = pid_grafica_x + ancho_barra;
      }
      grafica_x = 38 + map(tiempo_trabajo, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
      grafica_x = grafica_x - (grafica_x - grafica_x_anterior);
      grafica_y = 202 - map(temperatura_actual, 1, 300, 0, 144);
      tft.fillRect(38 + grafica_x, grafica_y, ancho_barra, 3, TFT_YELLOW);
      grafica_x_anterior = grafica_x + ancho_barra;
      finalizado = true;
      minutos_restantes = 0;
      segundos_restantes = 0;
      wdt_disable();
      tft.fillRect(162, 251, 162, 98, TFT_WHITE);
      tft.fillRect(24, 287, 120, 51, TFT_WHITE);
      tft.fillRect(104, 420, 210, 55, TFT_WHITE);
      Serial.print("-- REFLOW FINISHED --");
    }
    // SI TRABAJO FINALIZADO
    if (finalizado == true) {
      tft.drawNumber(temperatura_actual, 80, 315, 7);
      boton(107, 430, 105, 40, TFT_BLACK, "OK", TFT_WHITE, 4);
      if (calidad_pasada == true) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_BLUE);
        tft.drawString("Quality Stat", 235, 272, 4);
        tft.setTextColor(TFT_GREEN);
        tft.drawString("PASS", 240, 315, 4);
        Serial.print("Quality stat PASS");
      }
      if (calidad_pasada == false) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_BLUE);
        tft.drawString("Quality Stat", 235, 272, 4);
        tft.setTextColor(TFT_RED);
        tft.drawString("FAIL", 260, 315, 4);
        Serial.print("Quality test FAIL");
      }
      while (boton_accion != 1) {
        botones();
        counter = 0;
        borrar_pantalla = true;
        trabajo_iniciado = false;
        //attachInterrupt(digitalPinToInterrupt(DT), encoder, CHANGE);
        //GUARDAR EN LOG
      }
    }
  }
}
void pantalla_editar() { //ARREGLAR: evitar actualizacion de numeros y boton si no hay cambio en el encoder.

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
      pre_perfiles[i][3][0] = perfiles[i][3][0]; //temperatura 4
      pre_perfiles[i][4][0] = perfiles[i][3][0]; //temperatura 4
      pre_perfiles[i][0][1] = perfiles[i][0][1]; //tiempo 1
      pre_perfiles[i][1][1] = perfiles[i][1][1]; //tiempo 2
      pre_perfiles[i][2][1] = perfiles[i][2][1]; //tiempo 3
      pre_perfiles[i][2][1] = perfiles[i][3][1]; //tiempo 4
      pre_perfiles[i][4][1] = perfiles[i][4][1]; //tiempo
    }
    tft.fillRect(0, 200, 320, 80, TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.fillRect(0, 245, 320, 5, TFT_BLACK);
    tft.fillRect(0, 350, 320, 5, TFT_BLACK);
    tft.setTextColor(BROWN);
    tft.drawString("Temp", 55, 272, 4);
    tft.drawString("Seconds", 260, 272, 4);
    tft.drawString("STEP", 160, 272, 4);
    tft.drawString("Display", 46, 408, 2);
    boton(15, 420, 65, 25, TFT_BLUE, "C/Sec", TFT_WHITE, 2);
    tft.setTextColor(BROWN);
    sel_temperatura = false;
    sel_etapa = false;
    sel_tiempo = false;
    sel_perfil = false;
    sel_pendiente = false;
    boton_seleccion_anterior = false;
    guardado = false;
    mostrar_pendiente = false;
    counter = 0;
    counter_anterior = 0;
    id_perfil = 0;
    etapa = 0;
    boton_accion = 0;
    seleccion_menu_editar = 0;
    borrar_pantalla = false;
    refrescar_grafica = true;
    actualizar_seleccion=true;
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
  if (refrescar_grafica == true) {
    int tiempo = calcular_tiempo_total(id_perfil);
    int tiempo_ramp = perfiles[id_perfil][0][1];
    int tiempo_preheat = perfiles[id_perfil][1][1] + tiempo_ramp;
    int tiempo_ramp_reflow = perfiles[id_perfil][2][1] + tiempo_preheat;
    int tiempo_reflow = perfiles[id_perfil][3][1] + tiempo_ramp_reflow;
    int tiempo_cooldown = perfiles[id_perfil][4][1] + tiempo_reflow;
    int media1 = pre_perfiles[id_perfil][0][1] / 2;
    int media2 = (pre_perfiles[id_perfil][1][1] / 2) + pre_perfiles[id_perfil][0][1];
    int media3 = (pre_perfiles[id_perfil][2][1] / 2) + pre_perfiles[id_perfil][1][1] + pre_perfiles[id_perfil][0][1];
    int media4 = (pre_perfiles[id_perfil][3][1] / 2) + pre_perfiles[id_perfil][2][1] + pre_perfiles[id_perfil][1][1] + + pre_perfiles[id_perfil][0][1];
    int media5 = (pre_perfiles[id_perfil][4][1] / 2) + pre_perfiles[id_perfil][3][1] + pre_perfiles[id_perfil][2][1] + pre_perfiles[id_perfil][1][1] + pre_perfiles[id_perfil][0][1];

    //calcular grafica
    calcular_grafica(id_perfil);
    tft.setTextColor(BROWN);
    tft.setTextDatum(MR_DATUM);

    //obtener rectas
    tft.drawLine(38 + map(pre_perfiles[id_perfil][0][1], 1, tiempo, 1, 244), 45, 38 + map(pre_perfiles[id_perfil][0][1], 1, tiempo, 1, 244), 195, BROWN); // LINEA VERTICAL
    tft.drawLine(38 + map(pre_perfiles[id_perfil][1][1] + tiempo_ramp, 1, tiempo, 1, 244), 45, 38 + map(pre_perfiles[id_perfil][1][1] + tiempo_ramp, 1, tiempo, 1, 244), 195, BROWN); // LINEA VERTICAL
    tft.drawLine(38 + map(pre_perfiles[id_perfil][2][1] + tiempo_preheat, 1, tiempo, 1, 244), 45, 38 + map(pre_perfiles[id_perfil][2][1] + tiempo_preheat, 1, tiempo, 1, 244), 195, BROWN); // LINEA VERTICAL
    tft.drawLine(38 + map(pre_perfiles[id_perfil][3][1] + tiempo_ramp_reflow, 1, tiempo, 1, 244), 45, 38 + map(pre_perfiles[id_perfil][3][1] + tiempo_ramp_reflow, 1, tiempo, 1, 244), 195, BROWN); // LINEA VERTICAL
    //leyenda
    tft.setTextColor(BROWN);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Ramp Soak", 38 +  map(media1, 1, tiempo, 1, 244), 22, 2); //etapa 1
    tft.drawString("Pre-Heat", 38 + map(media2, 1, tiempo, 1, 244), 8, 2); //etapa 2
    tft.drawString("Ramp Peak", 38 +  map(media3, 1, tiempo, 1, 244), 22, 2); //etapa 3
    tft.drawString("Reflow", 38  + map(media4, 1, tiempo, 1, 244), 8, 2); //etapa 4
    tft.drawString("Cooldown", 38  + map(media5, 1, tiempo, 1, 244), 22, 2); //etapa 4

    //TEMPERATURA
    tft.setTextColor(TFT_BLUE);
    tft.setTextDatum(MR_DATUM);
    if (mostrar_pendiente == false) {
      tft.drawNumber(pre_perfiles[id_perfil][0][0], 38 +  map(pre_perfiles[id_perfil][0][1], 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][0][0] + 15, 1, 300, 0, 144), 4); //temp etapa 1
      if (pre_perfiles[id_perfil][0][0] != pre_perfiles[id_perfil][1][0]) {
        tft.drawNumber(pre_perfiles[id_perfil][1][0], 38 + map(pre_perfiles[id_perfil][1][1] + tiempo_ramp, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][1][0] + 15, 1, 300, 0, 144), 4);
      }//temp etapa 2
      tft.drawNumber(pre_perfiles[id_perfil][2][0], 38 + map(pre_perfiles[id_perfil][2][1] + tiempo_preheat, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][2][0] + 15, 1, 300, 0, 144), 4); //temp etapa 3
      if (pre_perfiles[id_perfil][2][0] != pre_perfiles[id_perfil][3][0]) {
        tft.drawNumber(pre_perfiles[id_perfil][3][0], 38 + map(pre_perfiles[id_perfil][3][1] + tiempo_ramp_reflow, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][3][0] + 15, 1, 300, 0, 144), 4); //temp etapa 4
      }
    } else { // si queremos mostrar pendiente es float
      tft.drawFloat(pre_perfiles[id_perfil][0][0] / pre_perfiles[id_perfil][0][1], 1, 38 +  map(pre_perfiles[id_perfil][0][1], 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][0][0] + 15, 1, 300, 0, 144), 4); //temp etapa 1
      if (pre_perfiles[id_perfil][0][0] != pre_perfiles[id_perfil][1][0]) {
        tft.drawFloat(pre_perfiles[id_perfil][1][0] / pre_perfiles[id_perfil][1][1], 1, 38 + map(pre_perfiles[id_perfil][1][1] + tiempo_ramp, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][1][0] + 15, 1, 300, 0, 144), 4);
      }//temp etapa 2
      tft.drawFloat(pre_perfiles[id_perfil][2][0] / pre_perfiles[id_perfil][2][1], 1, 38 + map(pre_perfiles[id_perfil][2][1] + tiempo_preheat, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][2][0] + 15, 1, 300, 0, 144), 4); //temp etapa 3
      if (pre_perfiles[id_perfil][2][0] != pre_perfiles[id_perfil][3][0]) {
        tft.drawFloat(pre_perfiles[id_perfil][3][0] / pre_perfiles[id_perfil][3][1], 1, 38 + map(pre_perfiles[id_perfil][3][1] + tiempo_ramp_reflow, 1, tiempo, 1, 244), 202 - map(pre_perfiles[id_perfil][3][0] + 15, 1, 300, 0, 144), 4); //temp etapa 4
      }

    }
    //TIEMPO
    tft.setTextDatum(MR_DATUM);
    tft.drawNumber(pre_perfiles[id_perfil][0][1], 38 + map(pre_perfiles[id_perfil][0][1], 1, tiempo, 0, 244), 215, 4); //tiempo etapa 1
    tft.drawNumber(pre_perfiles[id_perfil][1][1], 38 + map(pre_perfiles[id_perfil][1][1] + tiempo_ramp, 1, tiempo, 0, 244), 235, 4); //tiempo etapa 2
    tft.drawNumber(pre_perfiles[id_perfil][2][2], 38 + map(pre_perfiles[id_perfil][2][1] + tiempo_preheat, 1, tiempo, 0, 244), 215, 4); //tiempo etapa 3
    tft.drawNumber(pre_perfiles[id_perfil][3][3], 38 + map(pre_perfiles[id_perfil][3][1] + tiempo_ramp_reflow, 1, tiempo, 0, 244), 235, 4); //tiempo etapa 3
    tft.drawNumber(pre_perfiles[id_perfil][3][3], 38 + map(pre_perfiles[id_perfil][4][1] + tiempo_reflow, 1, tiempo, 0, 244), 215, 4); //tiempo etapa 3
    tft.setTextDatum(BL_DATUM); //NO BORRAR SI NO SE DESCUADRA TODO
    tft.setTextColor(TFT_GREEN);
    tft.drawRect(5, 62, 55, 19, TFT_GREEN);
    if (mostrar_pendiente == false) {
      tft.drawString("Temp C", 10, 80, 2); //Temp
    } else {
      tft.drawString("C/Sec", 15, 80, 2); //Temp
    }
    tft.drawRect(5, 218, 38, 19, TFT_GREEN);
    tft.drawString("Secs", 10, 235, 2); //Time

    refrescar_grafica = false;
  }
  tft.setTextDatum(MC_DATUM); //NO BORRAR SI NO SE DESCUADRA TODO


  if (etapa == 3) {
    tft.setTextColor(TFT_BLACK);
  } else {
    tft.fillRect(230, 328, 90, 20, TFT_WHITE);
  }

  //Escribir datos
  if (guardado == false) {
    if (sel_perfil == false && sel_temperatura == false && sel_tiempo == false && sel_etapa == false && sel_pendiente == false) {
      if (counter < 0) {
        counter = 7;
      }
      if (counter > 7) {
        counter = 0;
      }
      seleccion_menu_editar = counter;
    }
    switch (seleccion_menu_editar) {
      case 0:
        if (actualizar_seleccion == true) {
          tft.setTextColor(TFT_GREEN);
          tft.setTextColor(TFT_BLUE);
          tft.drawString(textoetapa, 160, 315, 4);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.fillRect(225, 290, 90, 40, TFT_WHITE);
          tft.setTextColor(TFT_BLUE);
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.drawNumber(tiempo, 270, 315, 6);
          temperatura_actual = pre_perfiles[id_perfil][etapa][0];
          tft.fillRect(8, 280, 90, 60, TFT_WHITE);
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }

        break;

      case 1: //seleccion perfil
        if (actualizar_seleccion == true) {
          refrescar_grafica=true;
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
          
          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
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
        if (sel_perfil == true && sel_temperatura == false && sel_tiempo == false && sel_etapa == false && sel_pendiente == false) {
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
          pre_perfiles[id_perfil][3][0] = perfiles[id_perfil][3][0]; //temperatura 4
          pre_perfiles[id_perfil][4][0] = perfiles[id_perfil][4][0]; //temperatura 4
          pre_perfiles[id_perfil][0][1] = perfiles[id_perfil][0][1]; //tiempo 1
          pre_perfiles[id_perfil][1][1] = perfiles[id_perfil][1][1]; //tiempo 2
          pre_perfiles[id_perfil][2][1] = perfiles[id_perfil][2][1]; //tiempo 3
          pre_perfiles[id_perfil][3][1] = perfiles[id_perfil][3][1]; //tiempo 4
          pre_perfiles[id_perfil][4][1] = perfiles[id_perfil][4][1]; //tiempo 5
          
        }

        break;

      case 2: //seleccion temperatura
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawString(textoetapa, 160, 315, 4);
          tft.drawNumber(tiempo, 270, 315, 6);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.setTextColor(TFT_BLACK);
          tft.drawNumber(temperatura_actual, 55, 315, 6);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_temperatura = true;
          counter = pre_perfiles[id_perfil][etapa][0];
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
        }
        if (sel_perfil == false && sel_temperatura == true && sel_tiempo == false && sel_etapa == false && sel_pendiente == false) {
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
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);
          tft.drawNumber(tiempo, 270, 315, 6);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.setTextColor(TFT_BLACK);
          tft.drawString(textoetapa, 160, 315, 4);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          sel_etapa = true;
          counter = 0;
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
        }
        if (sel_perfil == false && sel_temperatura == false && sel_tiempo == false && sel_etapa == true && sel_pendiente == false) {
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
            refrescar_grafica = true;
          }
        }
        break;
      case 4: //seleccion tiempo
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);

          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.drawString(textoetapa, 160, 315, 4);
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLACK);
          tft.drawNumber(tiempo, 270, 315, 6);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
        if (etapa != 3) {
          if (boton_accion == 1 && boton_seleccion_anterior == false) {
            counter = pre_perfiles[id_perfil][etapa][1];
            sel_tiempo = true;
            boton_seleccion_anterior = true;
            tiempo_delay = millis();
            boton_accion = 0;
          }

          if (sel_perfil == false && sel_temperatura == false && sel_tiempo == true && sel_etapa == false && sel_pendiente == false) {
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
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);
          tft.drawNumber(tiempo, 270, 315, 6);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.drawString(textoetapa, 160, 315, 4);

          boton(107, 430, 105, 40, TFT_BLACK, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
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
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);
          tft.drawNumber(tiempo, 270, 315, 6);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.drawString(textoetapa, 160, 315, 4);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLACK, "Exit", TFT_WHITE, 2);
          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_ORANGE, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_ORANGE, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
        if (boton_accion == 1 && guardado == false) {
          borrar_pantalla = true;
          numero_pantalla = 1;
        }
        break;
      case 7://seleccion grados / pendiente
        if (actualizar_seleccion == true) {
          tiempo = pre_perfiles[id_perfil][etapa][1];
          tft.setTextColor(TFT_BLUE);
          tft.drawNumber(temperatura_actual, 55, 315, 6);
          tft.drawNumber(tiempo, 270, 315, 6);
          tft.drawString(nombre_perfil[id_perfil], 160, 380, 4);
          tft.drawString(textoetapa, 160, 315, 4);

          boton(107, 430, 105, 40, TFT_GREEN, "Save", TFT_WHITE, 4);
          boton(240, 420, 65, 25, TFT_BLUE, "Exit", TFT_WHITE, 2);

          if (mostrar_pendiente == false) {
            boton(15, 420, 65, 25, TFT_BLACK, "Temp", TFT_WHITE, 2);
          } else {
            boton(15, 420, 65, 25, TFT_BLACK, "C/Sec", TFT_WHITE, 2);
          }
          actualizar_seleccion = false;
        }
        if (boton_accion == 1 && boton_seleccion_anterior == false) {
          if (mostrar_pendiente == false) {
            counter = 0;
          } else {
            counter = 1;
          }
          boton_seleccion_anterior = true;
          tiempo_delay = millis();
          boton_accion = 0;
          sel_pendiente = true;
        }
        if (sel_perfil == false && sel_temperatura == false && sel_tiempo == false && sel_etapa == false && sel_pendiente == true) {
          if (boton_accion == 1 && boton_seleccion_anterior == true && (tiempo_delay + retardo_cambio_estado + 50) < millis()) {
            sel_pendiente = false;
            refrescar_grafica = true;
          }
          if (counter < 0 ) {
            counter = 1;
          }
          if (counter > 1) {
            counter = 0;
          }
          if (counter == 0) {
            boton(15, 420, 65, 25, TFT_RED, "Temp", TFT_WHITE, 2);
            mostrar_pendiente = false;
          } else {
            boton(15, 420, 65, 25, TFT_RED, "C/Sec", TFT_WHITE, 2);
            mostrar_pendiente = true;
          }
        }
        break;

    }
   
  }
 if (counter != counter_anterior) {
      actualizar_seleccion = true;
    }
    counter_anterior = counter;
}
void mostrar_estadistica(String nombre_archivo) {
  cuadro_dialogo(15, 50, 290, 360, DARK_WHITE);
  tft.drawBitmap(18, 200, imagen_reflow_small, 287, 176, TFT_BLACK);
  tft.fillRect(20, 370, 284, 10, DARK_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_RED);
  tft.drawString("nombre_archivo", 160, 80, 4);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_BLACK);
  tft.drawString("R^2:", 40, 110, 2);
  tft.drawString("Quality stat:", 40, 130, 2);
  tft.drawString("Max temp:", 40, 150, 2);
  tft.drawString("Min temp:", 175, 150, 2);
  tft.setTextColor(TFT_GREEN);
  tft.drawFloat(0.99, 2, 120, 110, 2);
  tft.drawString("PASS", 120, 130, 2);
  tft.drawFloat(240, 0, 120, 150, 2);
  tft.drawFloat(120, 0, 240, 150, 2);
  /*
    grafica_x = 35 + map(tiempo_trabajo, 0, tiempo_total, 0, 240); //map(value, fromLow, fromHigh, toLow, toHigh)
     grafica_x = grafica_x - (grafica_x - grafica_x_anterior);
     grafica_y = 349 - map(temperatura_actual, 1, 300, 0, 144);
     tft.fillRect(38 + grafica_x, grafica_y, ancho_barra, 3, TFT_YELLOW);
     grafica_x_anterior = grafica_x + ancho_barra;
  */
}
void pantalla_log() {
  id_archivo = 0;
  if (borrar_pantalla == true) {
    tft.fillScreen(TFT_WHITE);

    if (sd_disponible == true && archivo_disponible == true) {
      cuadro_dialogo(80, 130, 160, 160, TFT_BLUE);
      tft.setTextColor(TFT_WHITE);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Reading...", 150, 210, 4);
      //listar_archivos(root, 0);
      tft.fillScreen(TFT_WHITE);
      tft.fillRect(0, 0, 320, 40, BROWN);
      tft.fillRect(50, 40, 2, 385, BROWN);
      tft.fillRect(270, 40, 2, 385, BROWN);
      tft.setTextColor(TFT_GREEN);
      tft.drawString("Solder history", 160, 20, 4);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("<", 10, 22, 4);
      tft.drawString("A", 30, 22, 4);
      tft.drawString(">", 310, 22, 4);
      tft.drawString("B", 290, 22, 4);
      tft.drawLine(90, 70, 230, 70, DARK_GREY); //celdas cada 55 pixeles -1 para dibujar la barra
      tft.drawLine(0, 95, 320, 95, BROWN);
      tft.drawLine(90, 124, 230, 124, DARK_GREY);
      tft.drawLine(0, 150, 320, 150, BROWN);
      tft.drawLine(90, 179, 230, 179, DARK_GREY);
      tft.drawLine(0, 205, 320, 205, BROWN);
      tft.drawLine(90, 234, 230, 234, DARK_GREY);
      tft.drawLine(0, 260, 320, 260, TFT_BLACK);
      tft.drawLine(90, 289, 230, 289, BROWN);
      tft.drawLine(0, 315, 320, 315, TFT_BLACK);
      tft.drawLine(90, 344, 230, 344, BROWN);
      tft.drawLine(0, 370, 320, 370, TFT_BLACK);
      tft.drawLine(90, 399, 230, 399, BROWN);
      tft.drawLine(0, 425, 320, 425, TFT_BLACK);
      boton(107, 430, 105, 40, TFT_GREEN, "OK", TFT_WHITE, 4);
      boton(240, 430, 65, 25, TFT_BLUE, "Back", TFT_WHITE, 2);


      //-----------------DEBUG-----------------------------
      csv_resumen("", 3);
      Serial.println(freeRam());                                 //debug
      //Plantilla
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 73, 6); //DECENA
      tft.drawNumber(9, 40, 78, 4); //UNIDAD
      tft.setTextColor(TFT_BLUE);
      tft.drawString(datos[0], 160, 58, 2); //1
      tft.drawString(datos[1], 165, 83, 2);
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 68, 2);
      //fin plantilla
      tft.setTextColor(TFT_BLUE); //2
      tft.drawString(datos[0], 160, 113, 2); //1
      tft.drawString(datos[1], 165, 138, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 128, 6); //DECENA
      tft.drawNumber(9, 40, 133, 4); //UNIDAD
      Serial.println(freeRam());                                 //debug
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 123, 2);

      tft.setTextColor(TFT_BLUE); //3
      tft.drawString("lalalala", 160, 168, 2); //1
      tft.drawString("DATE: 12/02/2022 END: 12:34", 165, 193, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 183, 6); //DECENA
      tft.drawNumber(9, 40, 188, 4); //UNIDAD
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 178, 2);

      tft.setTextColor(TFT_BLUE); //4
      tft.drawString("lalalala", 160, 223, 2); //1
      tft.drawString("DATE: 12/02/2022 END: 12:34", 165, 248, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 238, 6); //DECENA
      tft.drawNumber(9, 40, 243, 4); //UNIDAD
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 233, 2);

      tft.setTextColor(TFT_BLUE);//5
      tft.drawString("lalalala", 160, 278, 2); //1
      tft.drawString("DATE: 12/02/2022 END: 12:34", 165, 303, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 293, 6); //DECENA
      tft.drawNumber(9, 40, 298, 4); //UNIDAD
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 288, 2);

      tft.setTextColor(TFT_BLUE);//6
      tft.drawString("lalalala", 160, 333, 2); //1
      tft.drawString("DATE: 12/02/2022 END: 12:34", 165, 358, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 348, 6); //DECENA
      tft.drawNumber(9, 40, 353, 4); //UNIDAD
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 343, 2);

      tft.setTextColor(TFT_BLUE); //7
      tft.drawString("lalalala", 160, 388, 2); //1
      tft.drawString("DATE: 12/02/2022 END: 12:34", 165, 413, 2);
      tft.setTextColor(DARK_GREY);
      tft.drawNumber(2, 18, 403, 6); //DECENA
      tft.drawNumber(9, 40, 408, 4); //UNIDAD
      tft.setTextColor(TFT_GREEN);
      tft.drawString("PASS", 296, 398, 2);
    } else {

      if (sd_disponible == false) {
        tft.setTextColor(TFT_BLACK);
        tft.drawString("No SD or USB connected", 160, 230, 4);
        boton(107, 430, 105, 40, TFT_BLACK, "OK", TFT_WHITE, 4);
      }
      if (archivo_disponible == false) {
        tft.setTextColor(TFT_BLACK);
        tft.drawString("No SD INDEX File found", 160, 230, 4);
      }
    }
    counter = 0;
    boton_accion = 0;
    anterior_numero_log = 0;
    borrar_pantalla = false;
    //id_archivo=0;

  }

  if (counter < 0) {
    counter = 8;
  }
  if (counter > 8) {
    counter = 0;
  }

  numero_archivo = counter;
  if (sd_disponible == true) {
    if (numero_archivo != anterior_numero_log) { //cambiar a counter != counter_anterior ????
      //desde los los 50 y, desde 40 x hasta 55px
      switch (numero_archivo) {
        case 0: //default
          boton(107, 430, 105, 40, TFT_GREEN, "OK", TFT_WHITE, 4);
          tft.fillRect(0, 40, 50, 55, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 73, 6); //DECENA
          tft.drawNumber(9, 40, 78, 4); //UNIDAD
          break;
        case 1:
          tft.fillRect(0, 371, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 403, 6); //7 DECENA
          tft.drawNumber(9, 40, 408, 4); //7 UNIDAD
          tft.fillRect(0, 40, 50, 55, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 73, 6); //DECENA
          tft.drawNumber(9, 40, 78, 4); //UNIDAD
          tft.fillRect(0, 96, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 128, 6); //2 DECENA
          tft.drawNumber(9, 40, 133, 4); //2 UNIDAD
          break;
        case 2:
          tft.fillRect(0, 40, 50, 55, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 73, 6); //DECENA
          tft.drawNumber(9, 40, 78, 4); //UNIDAD
          tft.fillRect(0, 96, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 128, 6); //2 DECENA
          tft.drawNumber(9, 40, 133, 4); //2 UNIDAD
          tft.fillRect(0, 151, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 183, 6); //3 DECENA
          tft.drawNumber(9, 40, 188, 4); //3 UNIDAD
          break;
        case 3:
          tft.fillRect(0, 96, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 128, 6); //2 DECENA
          tft.drawNumber(9, 40, 133, 4); //2 UNIDAD
          tft.fillRect(0, 151, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 183, 6); //3 DECENA
          tft.drawNumber(9, 40, 188, 4); //3 UNIDAD
          tft.fillRect(0, 206, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 238, 6); //4 DECENA
          tft.drawNumber(9, 40, 243, 4); //4 UNIDAD
          break;
        case 4:
          tft.fillRect(0, 151, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 183, 6); //3 DECENA
          tft.drawNumber(9, 40, 188, 4); //3 UNIDAD
          tft.fillRect(0, 206, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 238, 6); //4 DECENA
          tft.drawNumber(9, 40, 243, 4); //4 UNIDAD
          tft.fillRect(0, 261, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 293, 6); //5 DECENA
          tft.drawNumber(9, 40, 298, 4); //5 UNIDAD

          break;
        case 5:
          tft.fillRect(0, 206, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 238, 6); //4 DECENA
          tft.drawNumber(9, 40, 243, 4); //4 UNIDAD
          tft.fillRect(0, 261, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 293, 6); //5 DECENA
          tft.drawNumber(9, 40, 298, 4); //5 UNIDAD
          tft.fillRect(0, 316, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 348, 6); //6 DECENA
          tft.drawNumber(9, 40, 353, 4); //6 UNIDAD
          break;
        case 6:
          tft.fillRect(0, 261, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 293, 6); //5 DECENA
          tft.drawNumber(9, 40, 298, 4); //5 UNIDAD
          tft.fillRect(0, 316, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 348, 6); //6 DECENA
          tft.drawNumber(9, 40, 353, 4); //6 UNIDAD
          tft.fillRect(0, 371, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 403, 6); //7 DECENA
          tft.drawNumber(9, 40, 408, 4); //7 UNIDAD

          break;
        case 7:
          tft.fillRect(0, 316, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 348, 6); //6 DECENA
          tft.drawNumber(9, 40, 353, 4); //6 UNIDAD
          tft.fillRect(0, 371, 50, 54, TFT_BLACK); //celdas cada 55 pixeles -1 para dibujar la barra
          tft.setTextColor(TFT_WHITE);
          tft.drawNumber(2, 18, 403, 6); //7 DECENA
          tft.drawNumber(9, 40, 408, 4); //7 UNIDAD
          tft.fillRect(0, 40, 50, 55, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 73, 6); //DECENA
          tft.drawNumber(9, 40, 78, 4); //UNIDAD

          boton(107, 430, 105, 40, TFT_GREEN, "OK", TFT_WHITE, 4);
          break;
        case 8:
          boton(107, 430, 105, 40, TFT_BLACK, "OK", TFT_WHITE, 4);
          tft.fillRect(0, 371, 50, 54, TFT_WHITE);
          tft.setTextColor(DARK_GREY);
          tft.drawNumber(2, 18, 403, 6); //7 DECENA
          tft.drawNumber(9, 40, 408, 4); //7 UNIDAD
          break;
      }
    }
    if (numero_archivo == 8 || numero_archivo == 0) {
      if (boton_accion == 1) {
        mostrar_estadistica("Nombre de prueba");
        boton_accion = 0;
        while (boton_accion == 0) {
          botones();
          //numero_pantalla = 1;
          //borrar_pantalla = true;
        }
        borrar_pantalla = true;
      }
    }
    anterior_numero_log = numero_archivo;
  } else {
    if (boton_accion == 1) {
      numero_pantalla = 1;
      borrar_pantalla = true;
    }
  }
}
void inicializar_pantalla() {

  borrar_pantalla = true;
  tft.init();
  tft.setRotation(0);
  tft.invertDisplay(0);
  counter = 0;
  numero_pantalla = 1;
  tft.fillScreen(TFT_WHITE);
}
void pantalla() {
  //Si interrupcion no usar funcion botones
  botones();
  encoder(); //debug

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
    case 4: //pantalla log
      pantalla_log();
      break;
    case 5: //pantalla ajustes
      pantalla_ajustes();
      break;
  }
}
