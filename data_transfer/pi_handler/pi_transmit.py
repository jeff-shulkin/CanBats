import sys
import serial
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


# LOTS OF WORK NEEDS TO BE DONE ON THIS
elif cmd == 'GET_LEAF_DATA':    # only ran by central node
    ser.write(1)
    while not ser.in_waiting(): # wait until the atmega acknowledges
        pass
    
    if ser.read() != 1: # communication error
        print("message not acknowledged properly, aborting")
    else:
        data = open("new_data.csv",'wb')

        buff = bytearray()          # hold onto the last four bytes
        stopcode = 'STOP'.encode()  # when we see this, we're done

        while True:
            while not ser.in_waiting(): # continually wait for another byte
                pass
            buff.append(ser.read())
            if len(buff) > 4:           # wait until buffer is full
                data.write(buff[0])
                buff = buff[1:]         # discard old byte
                if buff == stopcode:    # stop condition met
                    data.close()
                    break





ser.close()