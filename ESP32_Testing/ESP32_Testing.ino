#include <Wire.h>

// ---------------------------------------------------------------
// Pin definitions for UART1 (Serial1)
// ---------------------------------------------------------------
const int rx2 = 18;
const int tx2 = 17;


// ---------------------------------------------------------------
// I2C pins (currently unused since Wire.begin() is commented out)
// ---------------------------------------------------------------
const int sda = 21;
const int scl = 22;
int Addr = 12;

// ---------------------------------------------------------------
// General Variables
// ---------------------------------------------------------------
const byte STATUS_OK = 200;
const byte ERR = 204;
const unsigned long timeout = 200;


// Reads a single byte, but gives up after timeoutMs if nothing arrives.
// Returns false on timeout instead of blocking forever.
bool readByteWithTimeout(Stream &serial, byte &result, unsigned long timeoutMs) {
  unsigned long began = millis(); //grabs current clockTime
  while (!serial.available()) {
    if (millis() - began >= timeoutMs) return false; //if timeout exceeds, return fail
  }
  result = serial.read(); //records a read to the pointed value
  return true;
}

// ---------------------------------------------------------------
// Sends a data packet of two bytes, then waits for a return succes/fail
// ---------------------------------------------------------------
bool sendPacket(byte message, Stream &serial) {
  byte STATUS; //flag for wether or not the packet sent cleanly
  
  for(int i = 0; i < 10; i++){ //will garuntee at least 10 attempts to send a valid packet
    serial.write(message);
    serial.write('\n');
  
    while(!serial.available()); //waits for a status update to "re-sync" a bad send
    if(!readByteWithTimeout(serial, STATUS, timeout)) continue;
    if(STATUS == STATUS_OK){ 
      return true; //if successful, return the success
    }
  }
  return false; //else, return a bad send. for later local logic
}

// ---------------------------------------------------------------
// recieves a two byte packet and sends success/fail based on status
// to request a new packet up to 10 times.
// ---------------------------------------------------------------
int readPacket(Stream &serial) {
  byte candidate;
  byte next;

  for (int attempt = 0; attempt < 10; attempt++) {
    if (!readByteWithTimeout(serial, candidate, timeout)) { //reads a recieved character to canidate if one exists
      continue; //otherwise, moves on
    }
    while (true) {
      if (!readByteWithTimeout(serial, next, timeout)) { //reads a recieved character into the new buffer if one exists.
        break; //otherwise, escape the loop
      }
      if (next == '\n') { //if the next recieved character is valid
        serial.write(STATUS_OK);
        return (int)candidate;
      } 
      else {
        candidate = next; // slide forward, keep scanning
      }
    }

    serial.write(ERR);
  }

  return ERR;
}

void setup() {
  // .begin(baudrate, config, RX_PIN, TX_PIN)
  // Config SERIAL_8N1 = 8 data bits, No parity, 1 stop bit
  Serial1.begin(19200, SERIAL_8N1, rx2, tx2);

  //only ensures that begin ran successfully, harmless to have but likely redundant
  while (!Serial1);

  // I2C init left disabled on purpose -- uncomment if/when needed
  // Wire.begin(sda, scl, Addr);

  delay(500);
}

void loop() {
  int packetData;
  if(Serial1.available()){
    packetData = readPacket(Serial1);
    sendPacket((byte)packetData, Serial1);
  }
}
