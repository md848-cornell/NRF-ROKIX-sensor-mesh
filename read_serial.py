import serial
import time

ser = serial.Serial(
    port='/dev/ttyUSB0',\
    baudrate=115200,\
    #parity=serial.PARITY_NONE,\
    #stopbits=serial.STOPBITS_ONE,\
    #bytesize=serial.EIGHTBITS,\
        timeout=0)

print("connected to: " + ser.portstr)

count=1

while True:
    line = ser.readline()
    if line != b'':
        print(count, ':', line )
    count = (count+1) % 10000
    time.sleep(0.01)

ser.close()
