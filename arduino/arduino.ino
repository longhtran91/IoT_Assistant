#include <Wire.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>   //to use LCD function download this library from arduino site

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); //create an object for LCD
int redLightPin = 13;
bool newData = false;
int data[2];

void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Serial.begin(115200);           /* start serial for debug */
  lcd.begin(16, 2);      //initialize LCD
  lcd.print("hello!");
  pinMode(redLightPin, OUTPUT);
}

void loop() {
  delay(100);
  if (newData) {
    newData = false;
    processData();
  }
}

// function that executes when data is received from master
void receiveEvent(int howMany) {
  int numBytes = 50;
  char data_buffer[numBytes];
  
  if ( howMany > 0 && howMany < numBytes) // safety check
  {
    for (int i = 0; i < howMany; ++i)
    {
      data_buffer[i] = Wire.read();
    }
    data_buffer[howMany] = '\0';
    Serial.println(data_buffer);
    newData = true;
    char * token ;
    token  = strtok(data_buffer, "-");
    data[0] = atoi(token);
    token = strtok(NULL, "-");
    data[1] = atoi(token);
  }  
}


void processData() {
  if (data[0] == 0) {
    digitalWrite(redLightPin, data[1]);
  } else if (data[0] == 1) {
    writeTimerToLCD(data[1]);
  }
}

void writeTimerToLCD(int durationInSeconds) {
  unsigned long lastTime = 0;
  lastTime = millis();
  lcd.clear();
  String s = "Timer: ";
  lcd.print(s + String(durationInSeconds));
  while (durationInSeconds > 0)
  {
    if (millis() - lastTime >= 1000)
    {
      lastTime = millis();
      --durationInSeconds;
      lcd.clear();
      lcd.print(s + String(durationInSeconds));
    }
  }
  lcd.clear();
  lcd.print("Timer is up");
}
