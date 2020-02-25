#include <Wire.h>


void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Serial.begin(115200);           /* start serial for debug */
}

void loop() {
  delay(100);
}


// function that executes when data is received from master
void receiveEvent() {
  String data = "";
  while (0 < Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    data += c;
  }
  Serial.write(data.c_str());           /* print the request data */
}
