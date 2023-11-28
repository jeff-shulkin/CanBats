import signal
import sys
import serial
import json
import struct
from time import sleep

ser = serial.Serial("/dev/ttyS0", 9600) # Open port at 9600 baud

def send_data():
    data = open('outgoing_data.csv')

    for line in data:
        # wait for arduino ready
        

        vals = line.strip().split(',')
        time_bytes = int(vals[0]).to_bytes(4, 'little')
        batID_byte = int(vals[1]).to_bytes(1)
        confidence_bytes = struct.pack('f', float(vals[2]))

        # Send: time, batID, confidence
        print(f"sending data: {int(vals[0])}, {int(vals[1])}, {float(vals[2])}")
        print(f"raw data: {time_bytes.hex()}, {batID_byte.hex()}, {confidence_bytes.hex()}")
#        print("byte is of type",type(time_bytes[0]))
        ser.write(time_bytes)
        ser.write(batID_byte)
        ser.write(confidence_bytes)
        
    # send the stopcode
    ser.write(b'JUNK')
    ser.write(b'\xFF')
    ser.write(b'JUNK')

    data.close()

def receive_data():
    with open('bat_species.json') as bf:
        bat_species = json.load(bf)
    with open('node_locations.json') as nf:
        node_locations = json.load(nf)
    data = open("new_data.csv",'a')
    raw_read = ser.read()
    leaf_id = int.from_bytes(raw_read)
    print("got leaf id:",leaf_id, type(leaf_id))

    #ser.write(b'\x00')    # send ready
    #print("sent ready over serial")

    buff = ser.read(9)
    print("got a packet")
    print(f"raw buffer: {buff.hex()}")
    #print(' '.join(['{:02x}'.format(byte) for byte in buff]))

    # 0-3 are time, 4 is species ID, 5-8 are confidence
    otime = int.from_bytes(buff[:4], byteorder='big')
    speciesID = buff[4]
    loc = node_locations[leaf_id]
    confidence = struct.unpack('f',buff[5:9])[0]
    line_out = f'{otime},{loc[0]},{loc[1]},{bat_species[speciesID]},{confidence}\n'
    data.write(line_out)
    print("wrote a line:",line_out)
    data.close()


if __name__ == '__main__':
    #ser.open()

    if len(sys.argv) > 1:   # RUN THIS SCRIPT WITH AN ARG ON THE CENTRAL NODE
        while 1:
            print("waiting for data")
            receive_data()

    else:
        send_data()

    ser.close()
