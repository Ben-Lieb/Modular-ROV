#include <SoftwareSerial.h>

int wireAddress;
const char type = 'g'; //specific class goes here, currently using g for general

void denoteClass(){
  Wire.write(type)
}

void setup(){
  // put your setup code here, to run once:
  test.begin(9600);
  Serial.begin(9600);
}

void loop() {
  wireAddress = 0 + 8;
  Wire.begin(wireAddress);
  Wire.onRequest(denoteClass())
}
