"""
This is the master script that will always be running on the leaf node's RPi
Its job is to take in serial commands and begin the respective processes

IMPORTANT: I need the ai script to handle a keyboard interrupt to ensure it closes the file it was writing to
"""

import subprocess
import sys
import signal
import serial
from time import sleep

SEND_LEAF_DATA = b'\x00'
START_AI = b'\x01'
STOP_AI = b'\x02'
START_RECORDING = b'\x03'
STOP_RECORDING = b'\x04'

ai_process = None
record_process = None

def signal_handler(signal, frame):  # kills all subprocesses upon keyboard interrupt
    if ai_process != None: # janky syntax but you (hopefully) get the picture
        ai_process.terminate()
        ai_process.wait()

    if record_process != None: # janky syntax but you (hopefully) get the picture
        record_process.terminate()
        record_process.wait()

    print("All subprocesses killed, exiting")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)    # set the keyboard interrupt handler

ser = serial.Serial('/dev/ttyS0', 9600)     # Open serial port

while 1:
    cmd = ser.read()

    if cmd == SEND_LEAF_DATA:
        subprocess.run(['python3','pi_handler.py'])

    elif cmd == START_AI:
        ai_process = subprocess.Popen(['ai command args'])

    elif cmd == STOP_AI:
        ai_process.terminate()
        ai_process.wait()
        ai_process = None

    elif cmd == START_RECORDING:
        record_process = subprocess.Popen(['echo', 'record command args'])
    
    elif cmd == STOP_RECORDING:
        record_process.terminate()
        record_process.wait()
        record_process = None