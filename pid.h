#include <User_Setup.h>
#include <TimerOne.h> 
#include "pins.h";
unsigned long tiempo_reflow=0, tiempo_reflow_anterior=0;
boolean abortado = false, finalizado=false;
int id_tipo_pcb, id_perfil;
int temperatura_actual;
boolean calidad_pasada, trabajo_iniciado;
void inicializar_pid() {
}
byte byte_temperatura(){
int i;
  byte d = 0;

  for (i = 7; i >= 0; i--) {
    digitalWrite(relog, LOW);
    delayMicroseconds(10);
    if (digitalRead(sdo)) {
      // set the bit to 0 no matter what
      d |= (1 << i);
    }

    digitalWrite(relog, HIGH);
    delayMicroseconds(10);
  }

  return d;
}
float leer_temperatura(int numero_sensor) {
  uint16_t v;
  
  switch (numero_sensor) {
    case 1:
      digitalWrite(cs1, LOW);
      break;
    case 2:
      digitalWrite(cs2, LOW);
      break;
    case 3:
      digitalWrite(cs3, LOW);
      break;
  }

  delayMicroseconds(10);

  v = byte_temperatura();
  v <<= 8;
  v |= byte_temperatura();
  
switch (numero_sensor) {
    case 1:
      digitalWrite(cs1, HIGH);
      break;
    case 2:
      digitalWrite(cs2, HIGH);
      break;
    case 3:
      digitalWrite(cs3, HIGH);
      break;
  }

  if (v & 0x4) {
    // uh oh, no thermocouple attached!
    return NAN;
    // return -100;
  }

  v >>= 3;
  //v= (v * 0.25) * 9.0 / 5.0 + 32; //farenheit
  return v*0.25;
}
float promedio_temperatura(){
  
}
void pid(int perfil){
  
}
void masuno(){
  tiempo_reflow++;
  temperatura_actual=leer_temperatura(1);
    if(abortado==true){
    Timer1.stop();
  }
  if(finalizado==true){
    Timer1.stop();
  }
}
void iniciar_timer_pid(){ //SOLO DEBUG.
//Sustituir temporizador por TIMER imprecision
  Timer1.initialize(1000000);      //Configura el TIMER en 8 Segundos
  Timer1.attachInterrupt(masuno); 
  if(abortado==true){
    Timer1.stop();
  }
}
