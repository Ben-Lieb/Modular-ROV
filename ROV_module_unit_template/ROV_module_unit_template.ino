#include <SoftwareSerial.h>
#include <Wire.h>

const char tester = 'm';
const char type = 'g'; //specific class goes here, currently using g for general

const int rx = 11;
const int tx = 12;
const int piPort = 13;

SoftwareSerial test(rx,tx);

bool notLast = false;
bool indexed = false;
bool confirmed = false;
bool first = false;

int index;
int wireAddress;
int finalIndex = -1;

void serialWrite(bool first, char message){
  if(first == true){
    test.end();
    digitalWrite(piPort, LOW);
    delay(100);
    Serial.println(message);
    delay(50);
    test.begin(9600);
    digitalWrite(piPort, HIGH);
  }
  else if(first != true){
    Serial.write(message);
  }
}
void serialWrite(bool first, int message){
  if(first == true){
    test.end();
    digitalWrite(piPort, LOW);
    delay(100);
    Serial.println(message);
    delay(50);
    test.begin(9600);
    digitalWrite(piPort, HIGH);
  }
  else if(first != true){
    Serial.write(message);
  }
}

void denoteClass(){
  Wire.write(type)
}

void setup() {
  // put your setup code here, to run once:
  //establishes the pins required for SoftwareSerial
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);
  pinMode(piPort, OUTPUT);

  /*if next to the raspberry Pi, tells the pi to keep the port shut
  so local ports can do their damn job*/
  digitalWrite(piPort, HIGH);
  
  /*call all customly used pins here, available pins are digital 2-10 and 13
  A0-A3, A6, and A7. establish their mode here as well*/
  /*data flows test -> serial down, and serial -> test up*/

  test.begin(9600);
  Serial.begin(9600);
  test.setTimeout(1000);
  Serial.setTimeout(1000);

  delay(100);
  test.write(tester);
  
  first = Serial.find('m');
  if(first == true){
    first = false;
  }
  else if(first != true){
    index = 0;
    first = true;
  }

  if(first == false){
    serialWrite(first, tester);
  }

  notLast = test.find(tester);
  
  //logic for sending indexes down the line.
  if(first == true){
    if(notLast == true){
      test.write((index + 1));
    }
    else if(notLast != true){
      finalIndex = 0;
    }
  }
  else if(first != true){
    while(indexed == false){
      if(Serial.available() > 0){
        index = Serial.read();
      }
      if(index > 0 && notLast == true){
        test.write((index + 1));
        indexed = true;
      }
      else if(index > 0 && notLast != true){
        finalIndex = index;
        indexed = true;
      }
    }
  }

  if(finalIndex == -1){
    do{
      if(test.available() > 0){
      finalIndex = test.read();
      confirmed = true;
      }
    }while(confirmed == false);
  }
  wireAddress = finalIndex + 8;
  Wire.begin(wireAddress);
  Wire.onRequest(denoteClass())
  serialWrite(first, finalIndex);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //this loop should be empty, all code should be in response to Wire events.
}
