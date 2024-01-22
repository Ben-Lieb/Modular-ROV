#include <SoftwareSerial.h>
#include <Wire.h>

const char tester = 'a';
const char confirmation = 'r';

const int rx = 11;
const int tx = 12;

SoftwareSerial test(rx,tx);

bool last = false;
bool indexed = false;
bool confirmed = false;

int index;
int nextBoard;

char sensorClass[3];

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

//sends the board class to the master and sets the next call function
void denoteClass() {
  const char type = 's';
  Wire.write(type);
  Wire.onRequest(denoteSensor);
}

//sends the master the sensor classes on board
void denoteSensor(){
  //since this function can't take an input
  //this needs to be in this function as well
  sensorClass[0] = sensorType(A3);
  sensorClass[1] = 'n';
  sensorClass[2] = 'n';
  Wire.write(sensorClass, 3);
  Wire.onRequest(sensorData);
}

//sends the master the sensor data on the board
void sensorData(){
  float sensorInfo[3];
  byte binarySensor[(sizeof(sensorInfo)*sizeof(float))];
  for(int i = 0; i < sizeof(sensorClass); i++){
    sensorInfo[i] = sensorRead(i, ((i*3)+2), sensorClass[i]);
  }
  floatArrayToBinaryArray(sensorInfo, binarySensor, sizeof(sensorInfo));
  Wire.write(binarySensor, 12);
}

//determines sensor based on resistor values - current numbers are fudged slightly
//to account for the digital pin voltage difference from the true 5v rail
char sensorType(int analogPin){
  //first block is logic for the ohmeter
  const int Vin = 5;
  const float R1 = 9570.0;
  int raw;
  float Vout;
  float R2;
  float modifier = Vin/1024.0;
  char sensor;

  raw = analogRead(analogPin);
  Vout = raw * modifier;
  R2 = (Vout/((5 - Vout)/R1));
  Serial.println(R2);

  //this is the logic for determining sensor - t for tempurature, l for LED
  //you can add more sensors here
  if(4300 <= R2 && R2 <= 4900){
    sensor = 't';
    return sensor;
  }
  else if(500 <= R2 && R2 <= 600){
    sensor = 'l';
    return sensor;
  }
  else{
    sensor = 'n';
    return sensor;
  }
}

//sets pins for the led or tempurature sensors
//if adding more sensors, this also nees to be updated
//current format either 2 digital 1 analog, or 3 digital
void sensorSetup(char sensorType, int initialPin){
  if(sensorType == 't'){
    pinMode((initialPin + 2), OUTPUT);
    pinMode(initialPin, OUTPUT);
    digitalWrite((initialPin + 2), HIGH);
    digitalWrite(initialPin, LOW);
  }
  else if(sensorType == 'l'){
    pinMode(initialPin, OUTPUT);
    pinMode(initialPin + 1, OUTPUT);
    pinMode(initialPin + 2, OUTPUT);
    digitalWrite(initialPin, LOW);
    digitalWrite(initialPin + 1, HIGH);
    digitalWrite(initialPin + 2, LOW);
  }
  else{
  }
}

//returns the values from various sensors
//further sensors need to be updated here
float sensorRead(int index, int initialPin, char sensorClass){
  if(sensorClass == 't'){
    return ((((analogRead(A0 + index) / 1024.0) * 5.0) - .5) * 100);
  }
  else if(sensorClass == 'l'){
    if(digitalRead(initialPin + 1) == HIGH){
      digitalWrite(initialPin + 1, LOW);
      return 0;
    }
    else if(digitalRead(initialPin + 1) == LOW){
      digitalWrite(initialPin + 1, HIGH);
      return 1;
    }
  }
  else if(sensorClass == 'n'){
    return -1;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);

  //currently standing in for the 5v rail - will be fixed in PCB
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  test.begin(9600);
  Serial.begin(9600);
  Serial.setTimeout(1000);
  test.setTimeout(5000);

  //determines sensors on board
  sensorClass[0] = sensorType(A3);
  sensorClass[1] = 'n';
  sensorClass[2] = 'n';

  //establishes the pins for all sensors
  for(int i = 0; i < sizeof(sensorClass); i++){
    sensorSetup(sensorClass[i], ((i*3)+2));
    //(i*3)+2 iterates the digital pin by groups of three
  }

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

}
