#!/usr/bin/env python3

#needed for the serial communication w/ arduino
import serial
import time
import gpiozero
import smbus2

#open the communication ports to be used
arduino = serial.Serial('/dev/ttyAMA0', timeout=2)
I2C = smbus2.SMBus(1)
I2C.open(1)

#set up reset pin for boot - reset pin is active LOW, syncs up arduino's with the activation of this script
reset = gpiozero.LED(4)
reset.on()

#variable denotion
index = -1
moduleClass = []
bootConfirm = False

#reading serial data
def serialRead():
    data = arduino.readline().decode('utf_8').strip()
    return data

#writing serial data
def serialWrite(command):
    command = str(command)
    arduino.write(bytes(command, 'utf_8'))
    return

#waits for arduino index to return, stores index in var index
while bootConfirm == False:
    if serialRead() == "~":
        bootConfirm = True
    else:
        reset.off()
        time.sleep(.05)
        reset.on()

#sleep to let the ESP32 catch up
time.sleep(.1)

while index == -1:
    serialWrite((index + 1))
    if arduino.in_waiting() > 0:
        index = serialRead()
    time.sleep(.05)
    

for i in range(index + 1):
    moduleClass.append(I2C.read_byte_data((i + 8), 0))
    print(moduleClass(i))




#Set timeout to never occur, wait until the index is returned
Modules =  open("module.txt", "w")

Modules.close()
arduino.close()