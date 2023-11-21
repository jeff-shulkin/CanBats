"""
This is the master script that will always be running on the central node's RPi
Its job is to take in serial commands and begin the respective processes

IMPORTANT: I need the ai script to handle a keyboard interrupt to ensure it closes the file it was writing to
"""

import subprocess
import sys
import signal
import serial
from time import sleep

ai_process = 0

STORE_LEAF_DATA = b'\x00'
UPLOAD_DATA = b'\x02'

def signal_handler(signal, frame):  # kills all subprocesses upon keyboard interrupt
    print("All subprocesses killed, exiting")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)    # set the keyboard interrupt handler

ser = serial.Serial('/dev/ttyS0', 9600)     # Open serial port

while 1:
    cmd = ser.read()    # we wait forever for a serial command

    if cmd == STORE_LEAF_DATA:
        subprocess.run(['python3','data_transfer.py','a'])  # use .run to BLOCK this script's serial access until subprocess has ended

    elif cmd == UPLOAD_DATA:
        subprocess.Popen(['python3 upload_data.py new_data.csv'])   # we can use Popen because there's nothing to block
