import serial
import time
import gpiozero
import smbus2


I2C = smbus2.SMBus(1)
I2C.open(1)

print(I2C.read_byte_data((0 + 8), 0))

