#include <SoftwareSerial.h>
#include <Wire.h>

const char tester = 'a';
const char confirmation = 'r';

char sensorClass[/*if you need more than 3 sensors, this value will need to be updated across all boards.*/3];

//uses pointers, so "returns" nothing - converted values are assigned
//directly to input array
//arraySize is the size of the float array
void floatArrayToBinaryArray(float* floatArray, byte* binaryArray, int arraySize) {
  // Iterate through each float in the array
  for (int i = 0; i < arraySize; i++) {
    // Use a union to interpret the float as an array of bytes
    union {
      float floatValue;
      byte byteArray[sizeof(float)];
    } floatConverter;

    floatConverter.floatValue = floatArray[i];

    // Copy the binary representation to the provided array
    for (int j = 0; j < sizeof(float); j++) {
      binaryArray[i * sizeof(float) + j] = floatConverter.byteArray[j];
    }
  }
}

//denotes the module class and sets the next call function
void denoteClass() {
  const char type = '/*class goes here*/';
  Wire.write(type);
  Wire.onRequest(denoteSpeed);
}

//denotes the "sensors" on board, sets the next call funtion
void denoteSensors() {
  sensorClass[0] = /*onboard sensors are defined here*/;
  sensorClass[1] = /*sensors correlate to data that will be returned to the master board*/;
  sensorClass[2] = /*if you need more than three, that needs to also be updated in the master*/;
  Wire.write(sensorClass, 3);
  Wire.onRequest(readSensor);
}
//and on all other boards, so sensor call returns are standardized

void reportSensorValues(){
  float sensorValues[3];
  byte binarySpeed[(sizeof(motorSpeed)*sizeof(float))];
  
  sensorValues[0] = readSensor(/*general inputs here*/);
  sensorValues[1] = /*the inputs that will likely be baord specific*/;
  sensorValues[2] = /*null values for sensor shall be defined as -1*/;

  floatArrayToBinaryArray(motorSpeed, binarySpeed, sizeof(motorSpeed));
  Wire.write(binarySpeed, 12);
}

float readSensor(/*required custom inputs*/){
  /*sensor logic goes here, however it works*/
}

//takes the message sent by the master unit and
void recieveCommand(int byteCount){
  char recieved[byteCount];
  for(int i = 0; i < byteCount; i++){
    if(Wire.available() > 0){
      recieved[i] = Wire.read();
    }
  }
  /*custom code to parse a command from the master
  unit. howeever that logic works, it goes here*/
  else{
    delay(1);
  }
}

const int rx = 11;
const int tx = 12;

SoftwareSerial test(rx,tx);

bool last = false;
bool indexed = false;
bool confirmed = false;

int index;
int nextBoard;

void setup() {
  // put your setup code here, to run once:

  //establishes the pins required for SoftwareSerial
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);

  /*call all customly used pins here, available pins are digital 2-13
  A0-A3, A6, and A7. establish their mode here as well*/

  test.begin(9600);
  Serial.begin(9600);
  Serial.setTimeout(1000);
  test.setTimeout(5000);

  //tells the board ahead in the line that it's not the end
  delay(250);
  test.print(tester);

  //determines wether or not this board is the end
  if(Serial.find('a') == true){
    last = false;
  }
  else{
    last = true;
  }

  //waits for an index from the board ahead to establish where it is
  while(indexed == false){
    if(test.available() > 0){
      index = test.read();
      delay(50);
      if(index > -1){
        test.print(confirmation);
        indexed = true;
      }
    }
  }

  //sets the index for the next board and sets this boards I2C address
  nextBoard = index + 1;
  Wire.begin((index + 8));
  Wire.onRequest(denoteClass);
  Wire.onReceive(recieveCommand);

  //if this board is not last, sends the index to the next baord
  //if this board is last, sends the final index back up the chain
  if(last == false){
    while(confirmed == false){
      Serial.write(nextBoard);
      if(Serial.find('r') == true){
        confirmed = true;
      }
    }
  }
  else if(last == true){
    while(confirmed == false){
      test.write(index);
      if(test.find('r') == true){
        confirmed = true;
      }
    }
  }

  //waits for the final index to be sent back up, and repeats the final index until
  //receipt is confirmed
  if(last == false){
    indexed = false;
    confirmed = false;
    while(indexed == false){
      if(Serial.available() > 0){
        index = Serial.read();
        delay(50);
        if(index > -1){
          Serial.print(confirmation);
          indexed = true;
        }
      }
    }

    while(confirmed == false){
      test.write(index);
      if(test.find('r') == true){
        confirmed = true;
      }
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //this loop should be empty, all code should be in response to Wire events.
}