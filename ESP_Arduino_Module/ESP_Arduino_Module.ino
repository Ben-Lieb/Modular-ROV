#include <SoftwareSerial.h>
#include <Wire.h>

//module Specific information/pin info
char type = 'g'; //g for general rn, filler char
char board = 'a'; //a for arduino

//(ADC on 14-21, range 0-1023, available pins 2-10, 13, 14-17, 20-21)
int digitalPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 13}; 
int analogPins[] = {17, 20, 21};
int sensorDetectPins[] = {14, 15, 16};

//Initialize Serial pins/nessecary serial variables
const int rx = 11;
const int tx = 12;
SoftwareSerial test(rx,tx);

int Timeout = 1000;


//Information to initialize I2C (Format .begin(Data, Clocl, Addr);
int index = -1;
int Address;

bool last;
bool returnedIndex = false;

void setup(){
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);

  Serial.begin(115200);
  Serial.setTimeout(Timeout);
  test.begin(115200);
  test.setTimeout(Timeout);
  while(!Serial || !test); //waits until all serial ports are on
  
  //figure out if this is the last unit
  Serial.write("~");
  last = !test.find("~");
  delay(50);

  //read given index
  while(index == -1){
    if(Serial.available()){
      index = Serial.read();
      Address = index + 8;
      Wire.begin(Address);
    }
  }
  test.write((index + 1));

  //return final index to Pi
  while(returnedIndex == false){
    if(test.available()){
      Serial.write(test.read());
      returnedIndex = true;
    }
  }

}

void loop() {
  
}
