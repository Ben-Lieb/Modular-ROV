#include <Wire.h>

//module Specific information/pin info/infor for pi
const char type = 'g'; //g for general rn, filler char -> will define module class
const char board = 'e'; //e for Esp32

//(ADC on 1-20, range 0-4095 available pins 0-16, 19-21, 35-42, 45-48)
int digitalPins[] = {21, 35, 36, 37, 38, 39, 40, 41, 42, 45, 46, 47, 48}; 
int analogPins[] = {10, 11, 12, 13, 14, 15, 16, 19, 20};
int sensorDetectPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};


//Initialize Serial pins/nessecary serial variables
const int tx2 = 18;
const int rx2 = 17;

int Timeout = 1000;


//Information to initialize I2C (Format .begin(Data, Clock, Addr));
const int sda = 21;
const int scl = 22;
int moduleIndex = -1;
int Address;

bool last;
bool returnedIndex = false;


//tells the Pi what board is here
void denoteBoard(){
  Wire.write(board);
  Wire.onRequest(denoteType);
}

//tells the Pi what class the module is
void denoteType(){
  Wire.write(type);
  Wire.onRequest(controlInfo);
}

//tells the Pi what is on the module
void controlInfo(){

}

/*logic for determining what utility is on the board at a specific index - could be hardcoded, could be dynamic.
current stand in is logic for determining swappable sensors based on resistor value*/
char determineOnboard(int analogPin){
  const int Vin = 5;
  const float R1 = 10000;
  int raw;
  float vout;
  float R2;
  float modifier = 4096;
  char Onboard;

  raw = analogRead(analogPin);
  vout = raw * modifier;
  R2 = vout/((5-vout)/R1);

  if(R2 == 100){
    delay(1);
  }
  else{
    Onboard = 'n'; //n for null result
  }

  return Onboard;
}


void setup(){
  pinMode(tx2, OUTPUT);
  pinMode(rx2, INPUT);

  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(Timeout);
  Serial1.begin(115200, SERIAL_8N1, rx2, tx2);
  Serial1.setTimeout(Timeout);
  while(!Serial || !Serial1); //waits until all serial ports are on
  
  //figure out if this is the last unit
  Serial.write('~');
  last = !Serial1.find('~'); 
  delay(50);

  //read given index
  while(moduleIndex == -1){
    if(Serial.available()){
      moduleIndex = Serial.read();
      Address = moduleIndex + 8;
      Wire.begin(sda, scl, Address);
    }
  }

  //if not last, print index down the chain, if is last, return index back up the chain
  if(last != true){
    Serial1.write((moduleIndex + 1));
  }
  else if(last == true){
    Serial.write(moduleIndex);
  }
  
  //return final index to Pi
  while(returnedIndex == false){
    if(Serial1.available()){
      Serial.write(Serial1.read());
      returnedIndex = true;
    }
  }
}

void loop() {
  
}
