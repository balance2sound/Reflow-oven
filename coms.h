#include <SPI.h>
#include <SD.h>
#include "pins.h"
File root;
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
void inicializar_coms() {
  Serial.begin(19200);

  if (!SD.begin(cs_sd)) {
    Serial.println("SD initialization failed!");
  } else {
    Serial.println("SD FILES:");

    root = SD.open("/");

    printDirectory(root, 0);

    Serial.println("done!");
  }
}
