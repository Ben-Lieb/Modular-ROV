#include <Wire.h>

//Initialize Serial
const int tx1 = 43;
const int rx1 = 44;
const int tx2 = 17;
const int rx2 = 18;

char Data = 'f';

//Information to initialize I2C (Format .begin(Data, Clocl, Addr);
const int sda = 21;
const int scl = 22;
int Addr = 12;

void setup(){
    Serial.begin(9600);
    Serial1.begin(115200, SERIAL_8N1, RX1, TX1);
    Serial2.begin(115200, SERIAL_8N1, RX2, TX2);
    while(!Serial || !Serial1 || !Serial2);

    Serial.print("startup Successful");

    //Wire.begin(sda, scl, Addr);
    
    delay(500);
}

void loop() {
  Serial1.write(Data);
    if(Serial1.available()){
      Serial.println("data on line 1");
      Data = Serial1.read();
      delay(500);
      Serial1.write(Data);
      Serial.println(Data);
    }

    if(Serial2.available()){
      Serial.println("data on line 2");
      Data = Serial2.read();
      delay(500);
      Serial2.write(Data);
      Serial.println(Data);
    }
}
