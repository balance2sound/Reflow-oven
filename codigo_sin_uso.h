
/*
  const int buttonPin = 0; // input button pin number
  const unsigned long longPressThreshold = 5000; // the threshold (in milliseconds) before a long press is detected
  const unsigned long debounceThreshold = 50; // the threshold (in milliseconds) for a button press to be confirmed (i.e. not "noise")

  unsigned long buttonTimer = 0; // stores the time that the button was pressed (relative to boot time)
  unsigned long buttonPressDuration = 0; // stores the duration (in milliseconds) that the button was pressed/held down for

  boolean buttonActive = false; // indicates if the button is active/pressed
  boolean longPressActive = false; // indicate if the button has been long-pressed

  void setup() {
  pinMode(buttonPin, INPUT); // set the button pin as an input

  // Start serial debugging
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial debugging started");
  }

  void loop() {

  // if the button pin reads LOW, the button is pressed (negative/ground switch)
  if (digitalRead(buttonPin) == LOW)
  {
  // mark the button as active, and start the timer
  if (buttonActive == false)
  {
  buttonActive = true;
  buttonTimer = millis();
  }

  // calculate the button press duration by subtracting the button time from the boot time
  buttonPressDuration = millis() - buttonTimer;

  // mark the button as long-pressed if the button press duration exceeds the long press threshold
  if ((buttonPressDuration > longPressThreshold) && (longPressActive == false))
  {
  longPressActive = true;
  Serial.print("Long press detected: ");
  Serial.println(buttonPressDuration);
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
  if (buttonPressDuration > debounceThreshold)
  {
  Serial.print("Short press detected: ");
  Serial.println(buttonPressDuration);
  }

  // if the button press is less than our bounce threshold, we debounce (i.e. ignore as noise)
  else
  {
  Serial.print("Debounced: ");
  Serial.println(buttonPressDuration);
  }
  }

  // reset the button active status
  buttonActive = false;
  }
  }
  }
*/

/*
   int clock = 2;             // Define encoder pin A
  int data = 3;              // Define encoder pin B
  int count = 0;             // pre-init the count to zero
  int c = LOW;               // pre-init the state of pin A low
  int cLast = LOW;           // and make its last val the same - ie no change
  int d = LOW;               // and make the data val low as well

  void setup() {
  pinMode (clock,INPUT);  // setup the pins as inputs
  pinMode (data,INPUT);
  Serial.begin (9600);    // and give some serial debugging
  }

  void loop() {
  c = digitalRead(clock); // read pin A as clock
  d = digitalRead(data);  // read pin B as data

  if (c != cLast) {       // clock pin has changed value... now we can do stuff
    d = c^d;              // work out direction using an XOR
    if ( d ) {
      count--;            // non-zero is Anti-clockwise
    }else{
      count++;            // zero is therefore anti-clockwise
    }
    Serial.print ("Jog:: count:");
    Serial.println(count);
    cLast = c;            // store current clock state for next pass
  }
  }

  char* pBuffer;                              // Declare a pointer to your buffer.
    myFile = SD.open(F("fileName.txt"));        // Open file for reading.
    if (myFile)
    {
        unsigned int fileSize = myFile.size();  // Get the file size.
        pBuffer = (char*)malloc(fileSize + 1);  // Allocate memory for the file and a terminating null char.
        myFile.read(pBuffer, fileSize);         // Read the file into the buffer.
        pBuffer[fileSize] = '\0';               // Add the terminating null char.
        Serial.println(pBuffer);                // Print the file to the serial monitor.
        myFile.close();                         // Close the file.
    }
    // *** Use the buffer as needed here. ***
    free(pBuffer);      
*/

// encoder

//itoa añadir int a cadena de texto itoa( int valor, char * cadena_destino, int base )
