import subprocess
import serial

DATA_REQUEST = 0

# This script is meant to be ran upon serial interrupt

ser = serial.Serial("/dev/ttyS0", 9600) # Open port at 9600 baud

cmd = ser.read()
ser.write(cmd)

if cmd == DATA_REQUEST:
    subprocess.run(["python3","pi_transmit.py","SEND_DATA"])