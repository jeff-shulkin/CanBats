import subprocess
import serial

DATA_REQUEST = 0

# This script is meant to be ran upon serial interrupt

# Curious as to whether this can be combined with the previuos script
# Technically, all it takes is for the file to be executable by the user and the ISR

ser = serial.Serial("/dev/ttyS0", 9600) # Open port at 9600 baud

cmd = ser.read()
ser.write(cmd)

if cmd == DATA_REQUEST:
    subprocess.run(["python3","pi_transmit.py","SEND_DATA"])