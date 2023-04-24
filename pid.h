#include <User_Setup.h>
#include <TimerOne.h> 
#include "pins.h";
#include "pid_variables.h";
unsigned long tiempo_trabajo=0;
boolean abortado = false, finalizado=false;
int id_tipo_pcb, id_perfil;
float temperatura_objetivo,temperatura_actual;
float temperatura_ramp, temperatura_preheat, temperatura_reflow;
unsigned long tiempo_total, tiempo_ramp, tiempo_preheat, tiempo_cooldown, tiempo_reflow, tiempo_reflow2;
boolean calidad_pasada, trabajo_iniciado;
int etapa_pid;
float a,b_ramp,b_reflow,b_cooldown;

//variables pid
void inicializar_pid() { //BORRAR SI PRECISA
  etapa_pid=1;
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
  
 float temperatura;
 temperatura = (leer_temperatura(1)+leer_temperatura(2)+leer_temperatura(3))/3; // CAMBIAR PARA LEER SOLO 2
 return temperatura;
}
int pid(){ // cambiar a void
  switch (etapa_pid) {
    case 1: //parado
      a=leer_temperatura(1); //cambiar a promedio
      b_ramp=((temperatura_ramp-a)/tiempo_ramp);
      tiempo_reflow2=tiempo_reflow/2;
      etapa_pid=2;
      //una vez calculado pasa a siguiente paso
      break;
    case 2: //rampa
    if(tiempo_trabajo<=tiempo_ramp){
      temperatura_objetivo=(b_ramp*tiempo_trabajo)+a;
    }else{
    etapa_pid=3;
    //una vez finalizada rampa pasa a siguiente paso
    }
      break;
    case 3: //preheat
      if(tiempo_trabajo<=tiempo_preheat+tiempo_ramp){
      temperatura_objetivo=temperatura_preheat;
    }else{
    etapa_pid=4;
    a=leer_temperatura(1);                              //temp_reflow = 150 tiempo_reflow = 10 seg cambiar para leer todos los sensores
    b_reflow=((temperatura_reflow-a)/(tiempo_reflow/2)); //ejemplo:a=100 b_reflow=10 grados / segundo
    }
      break;
    case 4: //reflow
      if(tiempo_reflow2<=tiempo_reflow/2){ //editar mas
       temperatura_objetivo=(b_reflow*(tiempo_trabajo-(tiempo_preheat+tiempo_ramp)))+a;
      }else{
        a=temperatura_reflow;
        b_reflow=(-a/((tiempo_reflow/2)+tiempo_reflow*0.2)); //factor correccion tiempo 
        //debug
        /*
        if(temperatura_actual <= temperatura_reflow*0.8){
          etapa_pid=5;
        }
        */
      }
      break;
    case 5: //cooldown
      if(temperatura_actual <= 50){ //falta completar
          etapa_pid=6;
        }else{
        
        temperatura_objetivo=(-b_reflow*(tiempo_trabajo-(tiempo_preheat+tiempo_ramp+(tiempo_reflow/2))))+a;
        }
      break;
    case 6:
      
      break;
  }  
 Serial.print("Temp to reach: ");
 Serial.println(temperatura_objetivo);
 Serial.print("Actual Temp: ");
 Serial.println(temperatura_actual);
 temperatura_actual = leer_temperatura(1);
  //Next we calculate the error between the setpoint and the real value
  PID_error = temperatura_objetivo - temperatura_actual;
  //Calculate the P value
  PID_p = kp * PID_error;
  //Calculate the I value in a range on +-3
  if(-3 < PID_error <3)
  {
    PID_i = PID_i + (ki * PID_error);
  }

  //For derivative we need real time to calculate speed change rate
  timePrev = Time;                            // the previous time is stored before the actual time read
  Time = millis();                            // actual time read
  elapsedTime = (Time - timePrev) / 1000; 
  //Now we can calculate the D calue
  PID_d = kd*((PID_error - previous_error)/elapsedTime);
  //Final total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  //We define PWM range between 0 and 255
  if(PID_value < 0)
  {    PID_value = 0;    }
  if(PID_value > 255)  
  {    PID_value = 255;  }
  //Now we can write the PWM signal to the mosfet on digital pin D3
  previous_error = PID_error;
  return PID_value;
  //return 255-PID_value;
  
  
  
}
void masuno(){
  if(abortado==true){
    Timer1.stop();
    temperatura_objetivo=0;
  }
  tiempo_trabajo++;
  temperatura_actual=leer_temperatura(1);
  if(finalizado==true){
    Timer1.stop();
  }
  pid();
}
void iniciar_timer_pid(){
//Sustituir temporizador por TIMER imprecision
  Timer1.initialize(1000000);      //Configura el TIMER en 1 Segundos
  Timer1.attachInterrupt(masuno); 
}
