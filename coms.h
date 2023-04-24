#include <SPI.h>
#include <SD.h>
#include <CSV_Parser.h>
#include "pins.h"
boolean sd_disponible, serial_disponible, archivo_disponible=true;
char *nombre_archivo;
char log_index;
char **datos;
File archivo;
void listar_archivos(File dir, int numTabs) {
int a=0;
    while (true) {

    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    nombre_archivo=entry.name();
    Serial.print(nombre_archivo);
    a++;
    if (entry.isDirectory()) {
      Serial.println("/");
     listar_archivos(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
    
  }
  
}
char* leer_archivo(String nombre_archivo, int numero_linea){
  char* datos_archivo;
archivo = SD.open(nombre_archivo);        // Open file for reading.
free(datos_archivo);
Serial.write("Loading ");
Serial.println(nombre_archivo);
char salida;
    if (archivo)
    {
        unsigned int fileSize = archivo.size();  // Get the file size.
        datos_archivo = (char*)malloc(fileSize + 1);  // Allocate memory for the file and a terminating null char.
        archivo.read(datos_archivo, fileSize);         // Read the file into the buffer.
        datos_archivo[fileSize] = '\0';               // Add the terminating null char.              // Print the file to the serial monitor.
        archivo.close();                         // Close the file.
        Serial.write(datos_archivo);
    }
    return datos_archivo;
}  
void inicializar_coms() { //ARREGLAR: Hacer handshake para verificar que el puerto esta conectado
  Serial.begin(19200);
  if(!Serial){
  serial_disponible = false;
 }else{
  serial_disponible = true;
  Serial.println("REFLOW Machine Ready");
 }
  if (!SD.begin(cs_sd)) {
    Serial.println("No SD Card? SD card initiation failed");
    sd_disponible=false;
  } else {
    sd_disponible=true;
    Serial.println("SD Ready");
  }
}
void csv(){
  
 CSV_Parser file_data(/*format*/ "sdd", /*has_header*/ true, /*delimiter*/ ',');

  // The line below (readSDfile) wouldn't work if SD.begin wasn't called before.
  // readSDfile can be used as conditional, it returns 'false' if the file does not exist.
  if (file_data.readSDfile("/csv.csv")) {
  char **datos = (char**)file_data["datos"];
int16_t *valores_x = (int16_t*)file_data["valores_x"];
int16_t *valores_y  = (int16_t*)file_data["valores_y"];
    if (datos && valores_x && valores_y) {
      for(int row = 0; row < file_data.getRowsCount(); row++) {
        Serial.print("row = ");
        Serial.print(row, DEC);
        Serial.print(", datos = ");
        Serial.print(datos[row]);
        Serial.print(", valores x = ");
        Serial.println(valores_x[row], DEC);
        Serial.print(", valores y = ");
        Serial.println(valores_y[row], DEC);
        
      }
      Serial.println(datos[1]);
        
    } else {
      Serial.println("ERROR: At least 1 of the columns was not found, something went wrong.");
    }
  } else {
    Serial.println("ERROR: File called '/file.csv' does not exist...");
  }
 
}
void csv_resumen(String archivo, int linea){
  CSV_Parser file_data(/*format*/ "sdd", /*has_header*/ true, /*delimiter*/ ',');
 File csv_file = SD.open("/csv.csv"); // or FFat.open(f_name);
 file_data.parseLeftover();
while (csv_file.available()) {
    file_data << (char)csv_file.read();
}
  // The line below (readSDfile) wouldn't work if SD.begin wasn't called before.
  // readSDfile can be used as conditional, it returns 'false' if the file does not exist.
  
**datos = (char**)uint16_t(file_data["datos"]);
int16_t *valores_x = (int16_t*)file_data["valores_x"];
int16_t *valores_y  = (int16_t*)file_data["valores_y"];
datos=(char**)file_data[0];
valores_x =(int16_t*)file_data[1];
valores_y  = (int16_t*)file_data[2];
Serial.println(datos[0]);

    if (!datos && !valores_x && !valores_y) {
      Serial.println("ERROR: CSV incorrect format for read");
  } 
//poner codigo de error si no encuentra archivo 
}
void menu_serie(){
Serial.println("------------------------------");
Serial.println("|     Reflow Oven V 1.0       |");
Serial.println("|                             |");
Serial.println("------------------------------");
Serial.println("Press 1 for Reflow");   
Serial.println("Press 2 for Edit a Profile");
Serial.println("Press 3 for configuration");    
}
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
