#include <SoftwareSerial.h>
#include <Wire.h>

const char tester = 'a';
const char confirmation = 'r';

const int rx = 11;
const int tx = 12;

SoftwareSerial test(rx,tx);

bool confirmed = false;
bool indexed = false;

int index = 0;

//established these as global pointers - will be filled with
//class and sensor data during setup, as well as sized during setup
char* Classes;
char** Sensors;

//converts a binary array to a single float
float binaryArrayToFloat(byte* binaryArray) {
  // Use a union to interpret the byte array as a float
  union {
    float floatValue;
    byte byteArray[sizeof(float)];
  } floatConverter;

  // Copy the binary array to the union
  for (int i = 0; i < sizeof(float); i++) {
    floatConverter.byteArray[i] = binaryArray[i];
  }

  // Return the interpreted float value
  return floatConverter.floatValue;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);

  test.begin(9600);
  Serial.begin(9600);
  test.setTimeout(5000);

  //sends the started index (0) and waits for confirmation
  while(confirmed == false){
    Serial.println("Indexing start");
    test.write(index);
    if(test.find('r') == true){
      confirmed = true;
      Serial.println("confirmed");
    }
  }

  //once confirmed waits for final index
  Serial.println("waiting");
  while(indexed == false){
    if(test.available() > 0){
      index = test.read();
      delay(50);
      if(index > -1){
        test.print(confirmation);
        indexed = true;
        Serial.print("final index: ");
        Serial.println(index);
      }
    }
  }

  //generates the class and sensor arrays
  char Class[(index + 1)];
  //establishes the size of Classes
  Classes = new char[(index + 1)];
  char sensorArrays[(index + 1)][3];

  //establishes the first dimension of Sensors, while leaving the 2nd undefined
  Sensors = new char*[(index + 1)];
  //defines the second dimension as 3
  for (int i = 0; i < (index + 1); i++) {
    Sensors[i] = new char[3]; // Fixed second dimension at 3
  }

  //starts the I2C channel with itself as the master
  Wire.begin();
  Serial.println("wire began");
  for(int i = 0; i <= index; i++){
    int localIndex = i + 8;
    //requests the module class
    Wire.requestFrom(localIndex, 1);
    if(Wire.available() > 0){
      Class[i] = Wire.read();
    }
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(Class[i]);
  }
  
  for(int i = 0; i < sizeof(Class); i++){
    int localIndex = i + 8;
    //uses logic to determine the standard number of onboard sensors for a module
    //and requests the classes of all of them
    Wire.requestFrom(localIndex, 3);
    for(int j = 0; j < 3; j++){
      if(Wire.available() > 0){
        sensorArrays[i][j] = Wire.read();
      }
    }
  }
  for(int i = 0; i < sizeof(Class); i++){
    for(int j = 0; j < 3; j++){
      //if(sensorArrays[i][j] != 'n'){
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(sensorArrays[i][j]);
      //}
    }
  }

  //establishes Classes and Sensors as a global array with the same content as Class
  //may be removed later, but uh... not now, I don't wanna rewrite shit
  //could be done by replacing every instance of class w/ classes and sensorArrays w/ Sensors
  for(int i = 0; i < sizeof(Class); i++){
    Classes[i] = Class[i];
  }
  for(int i = 0; i < sizeof(Class); i++){
    for(int j = 0; j < 3; j++){
      Sensors[i][j] = sensorArrays[i][j];
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // these need to be re-established for logic post setup
  char motorCommand[2];
  float sensorValues[(index + 1)][3];

  //takes a serial input when given and sends it to all motor boards, for them to process
  //if elevators or rudders get added, this is where to do it
  if(Serial.available()){
    String recieved = "";
    recieved = Serial.readStringUntil('e');
    recieved.trim();
    for(int i = 0; i < sizeof(motorCommand); i++){
      motorCommand[i] = recieved.charAt(i);
    }

    for(int i = 0; i < sizeof(Classes); i++){
      int localIndex = i + 8;
      if(Classes[i] == 'm'){
        Wire.beginTransmission(localIndex);
        Wire.write(motorCommand, 2);
        Wire.endTransmission();
      }
    } 
  }
  
  //starts the sensor reading Loop - all current boards output a standard 3 data fields
  //any empty locations are filled with a null value - currently assigned as -1
  //won't need to be changed for future modules
  for(int i = 0; i < sizeof(Classes); i++){
    int localIndex = i + 8;

    //for translating form the recieved binary array back to a float
    byte binaryRecieve[12];
    byte binaryBuffer[sizeof(float)];
    float floatBuffer;
    
    Wire.requestFrom(localIndex, 12);
    if(Wire.available() > 0){
      for(int j = 0; j < 12; j++){
          binaryRecieve[j] = Wire.read();
      }
    }

    for(int j = 0; j < 3; j++){
      for(int k = 0; k < sizeof(binaryBuffer); k++){
        binaryBuffer[k] = binaryRecieve[(k + (4*j))];
      }
      floatBuffer = binaryArrayToFloat(binaryBuffer);
      sensorValues[i][j] = floatBuffer;
    }
  }
  
  //prints sensors next to their corresponding data
  for(int i = 0; i < sizeof(Classes); ++i){
    for(int j = 0; j < 3; ++j){
      if(Sensors[i][j] != 'n'){
        Serial.print(Sensors[i][j]);
        Serial.print(" : ");
        Serial.println(sensorValues[i][j]);
      }
    }
  }

  delay(1000);
}
