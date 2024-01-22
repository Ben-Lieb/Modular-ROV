#include <SoftwareSerial.h>
#include <Wire.h>

const char tester = 'a';
const char confirmation = 'r';

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

//denotes the module class and sets the next call function
void denoteClass() {
  const char type = 'm';
  Wire.write(type);
  Wire.onRequest(denoteSpeed);
}

//denotes the "sensors" on board, sets the next call funtion
void denoteSpeed() {
  sensorClass[0] = 'm';
  sensorClass[1] = 'm';
  sensorClass[2] = 'n';
  Wire.write(sensorClass, 3);
  Wire.onRequest(reportSpeed);
}

//reports the speed of on board motors
void reportSpeed(){
  float motorSpeed[3];
  byte binarySpeed[(sizeof(motorSpeed)*sizeof(float))];
  
  motorSpeed[0] = findSpeed(2);
  motorSpeed[1] = findSpeed(4);
  motorSpeed[2] = -1;

  floatArrayToBinaryArray(motorSpeed, binarySpeed, sizeof(motorSpeed));
  Wire.write(binarySpeed, 12);
}

//finds speed of onboard motors - will have to be changed later, either becuase
//I'll have these stored as integers somewhere
//or this will include logic to turn microseconds into "max MPH"
//or some other reason that i don't know enough programming to fathom
float findSpeed(int pinIndex){
  if(digitalRead(pinIndex) == HIGH){
    return 1;
  }
  else if(digitalRead(pinIndex) == LOW){
    return 0;
  }
}

//takes the message sent by the master unit and
void motorSpeed(int byteCount){
  char recieved[byteCount];
  for(int i = 0; i < byteCount; i++){
    if(Wire.available() > 0){
      recieved[i] = Wire.read();
    }
  }
  if(recieved[0] == 'r'){
    if(recieved[1] == '0'){
      digitalWrite(2, LOW);
    }
    else if(recieved[1] == '1'){
      digitalWrite(2, HIGH);
    }
  }
  else if(recieved[0] == 'l'){
    if(recieved[1] == '0'){
      digitalWrite(4, LOW);
    }
    else if(recieved[1] == '1'){
      digitalWrite(4, HIGH);
    }
  }
  else if(recieved[0] == 'b'){
    if(recieved[1] == '0'){
      digitalWrite(2, LOW);
      digitalWrite(4, LOW);
    }
    else if(recieved[1] == '1'){
      digitalWrite(2, HIGH);
      digitalWrite(4, HIGH);
    }
  }
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
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);

  //setting up motor pins
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);

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
  Wire.onReceive(motorSpeed);

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
