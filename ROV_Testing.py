import serial
import time
import gpiozero
import smbus2

'''
I2C = smbus2.SMBus(1) #creates an I2C object
I2C.open(1) #opens that i2c opject

print(I2C.read_byte_data((0 + 8), 0)) #should technically retrieve i2c data from the address
'''

import serial
import time

STATUS_OK = 200
STATUS_ERR = 204
BYTE_TIMEOUT_S = 0.1  # matches BYTE_TIMEOUT_MS = 100 from the Arduino side

arduino = serial.Serial('/dev/serial0', 19200, timeout=BYTE_TIMEOUT_S) #creates a serial object, w/timeout and baudrate


def read_byte_with_timeout(s):
    """Returns int 0-255, or None if nothing arrived within the timeout."""
    data = s.read(1)  # blocks up to `timeout` seconds, returns b'' on timeout
    if len(data) == 0:
        return None
    return data[0]  # indexing a bytes object gives you an int directly


def read_packet(s, max_attempts=10):
    for _ in range(max_attempts):
        candidate = read_byte_with_timeout(s)
        if candidate is None:
            continue  # nothing arrived this attempt, try again

        while True:
            nxt = read_byte_with_timeout(s)
            if nxt is None:
                break  # no terminator in time, abandon this attempt
            if nxt == ord('\n'):
                s.write(bytes([STATUS_OK]))
                return candidate
            else:
                candidate = nxt  # slide forward, keep scanning

        s.write(bytes([STATUS_ERR]))

    return STATUS_ERR


def send_packet(message, s, max_attempts=10):
    for _ in range(max_attempts):
        s.write(bytes([message]))
        s.write(b'\n')

        status = read_byte_with_timeout(s)
        if status == STATUS_OK:
            return True
        # else: loop again and resend

    return False

while True:
    message = int(input("what do you want to send? "))
    send_packet(message, arduino)
    print(read_packet(arduino))



