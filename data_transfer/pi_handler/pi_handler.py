import signal
import sys
import serial
import json
import struct
from time import sleep
import RPi.GPIO as GPIO

DATA_REQUEST = 0
INTERRUPT_PIN = 25
NUM_LEAF_NODES = 1

def send_data():
    data = open('new_data.csv')

    for line in data:
        # wait for arduino ready
        while not ser.in_waiting:
            sleep(0.1)

        junk = ser.read()   # discard ready byte

        vals = line.strip().split(',')
        time_bytes = int(vals[0]).to_bytes(4)
        batID_bytes = int(vals[1]).to_bytes(4)
        confidence_bytes = struct.pack('!f', float(vals[2]))

        # Send: time (bigE), batID (one byte), confidence (bigE)
        for byte in time_bytes:
            ser.write(byte)
        ser.write(batID_bytes[3])
        for byte in confidence_bytes:
            ser.write(byte)
        
    # send the stopcode
    ser.write("JUNK")
    ser.write(0xFF)
    ser.write("JUNK")

    data.close()

def get_leaf_data():
    bat_species = json.load('bat_species.json')
    node_locations = json.load('node_locations.json')
    data = open("new_data.csv",'w')
    for node_id in range(NUM_LEAF_NODES):
        ser.write(0)
        ser.write(node_id)
        while not ser.in_waiting(): # wait for ack
            sleep(0.1)
        
        if ser.read() != 0: # communication error
            print("message not acknowledged properly, aborting")
            break

        buff = bytearray()
        stopcode = 0xFF  # when we see this, we're done

        while True:
            ser.write(0)    # send ready

            while len(buff) < 9:    # wait until we've collected 9 bytes
                # wait until a byte is available
                while not ser.in_waiting:
                    sleep(0.01)
                
                buff.append(ser.read())
            
            # all data has been collected from this node
            if buff[4] == stopcode:
                break
            
            # 0-3 are time (bigE), 4 is species ID, 5-8 are confidence (bigE)
            otime = buff[0]<<24 | buff[1]<<16 | buff[2]<<8 | buff[3]
            speciesID = buff[4]
            loc = node_locations[node_id]
            confidence = struct.unpack('!f',buff[5:9])[0]
            data.write(f'{otime},{loc[0]},{loc[1]},{bat_species[speciesID]},{confidence}\n')

            del buff[:]

    data.close()

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def serial_handle(channel):
    global ser
    cmd = ser.read()
    ser.write(cmd)

    if cmd == DATA_REQUEST:
        send_data()

if __name__ == '__main__':
    ser = serial.Serial("/dev/ttyS0", 9600) # Open port at 9600 baud
    ser.open()

    if len(argv) > 1:   # RUN THIS SCRIPT WITH AN ARG ON THE CENTRAL NODE
        get_leaf_data()

    else:
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(INTERRUPT_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

        GPIO.add_event_detect(INTERRUPT_PIN, GPIO.BOTH, 
                callback=serial_handle, bouncetime=50)
        
        signal.signal(signal.SIGINT, signal_handler)
        signal.pause()

    ser.close()