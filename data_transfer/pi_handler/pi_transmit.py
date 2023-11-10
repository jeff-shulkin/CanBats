import sys
import serial
import json
import struct
from time import sleep

ser = serial.Serial("/dev/ttyS0", 9600) # Open port at 9600 baud
ser.open()

cmd = sys.argv[1]   # the command being ran by the caller

if cmd == 'SEND_DATA':    # only rank by leaf nodes
    ser.write(0) # send acknowledge byte

    with open('new_data.csv', 'rb') as df:    # read the newly collected bat data
        byte = df.read(1)
        while byte:
            ser.write(byte) # send one byte at a time
            byte = df.read(1)
    
    ser.write(b'STOP')  # stop condition

elif cmd == 'GET_LEAF_DATA':    # only ran by central node

    bat_species = json.load('bat_species.json')
    node_locations = json.load('node_locations.json')
    data = open("new_data.csv",'w')
    num_nodes = 0
    for node_id in range(num_nodes):
        ser.write(0)
        ser.write(node_id)
        while not ser.in_waiting(): # wait for ack
            pass
        
        if ser.read() != 0: # communication error
            print("message not acknowledged properly, aborting")
            break

        buff = bytearray()
        stopcode = 0xFF  # when we see this, we're done

        while True:
            while len(buff) < 9:    # wait until we've collected 9 bytes
                # wait until a byte is available
                while not ser.in_waiting:
                    # sleep
                    pass
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



ser.close()