# Serial Commands  
This document covers the different serial commands between the Raspberry Pi and ATmega  

## Data Transmission Process

This is a breakdown of the entire process of data going from the leaf nodes to the server

1. central node's (cn) pi tells cn's atmega to get all the data
2. cn's atmega sends lora message to leaf node (ln)
3. ln's atmega receives lora message
4. ln's atmega tells ln's pi to send the new data
5. ln's pi sends the new data to ln's atmega
6. ln's atmega sends the new data to cn's atmega over lora
7. cn's atmega receives data byte by byte
8. cn's atmega sends data to cn's pi byte by byte
9. cn's pi writes that data to the csv file
10. cn's pi uploads that csv file to the server

## Transmission Commands

### Pi to Pi
- `'SEND_DATA'`: send the recently collected bat data to the ATmega for transmission
- `'GET_LEAF_DATA'`: get all leaf data, stores data in csv file

### PI to ATmega
- `GET_LEAF_DATA (0x00)`: Sends ATmega a leaf node ID and requests that the ATmega sends the PI the data from that leaf node

### ATmega to Pi
- `DATA_REQUEST (0x00)`: request the the recently collected bat data

### ATmega to ATmega (LoRa)
LoRa commands are two bytes. The first is the target node ID and the second is the command. Upon receiving, the target node first transmits its own ID as acknowledgement
- `DATA_REQUEST (0x00)`: requests that the receiving node sends its recently collected bat data

## Testing Setup

Pin config:
- Make sure you've correctly configured `NUM_LEAF_NODES` in `pi_handler.py`
- Connect GPIO-25 on the Pi to d4 on the Arduino
- ~~Connect GPIO-19 on the Pi to d5 on the Arduino~~
- Pi and Arduino serial should be connected
- LoRa modules should be connected to Arduino

On the **leaf nodes**, have in a directory `pi_handler.py` and a csv file named `new_data.csv`. This CSV file should have lines formatted as follows:  
`<uint32>,<uint8>,<float32>`  
Execute the Python script with no arguments.

On the **central node**, have in a directory `pi_handler.py`, `bat_species.json`, and `node_locations.json`. For the node locations file, create a list of tuples, each having two float values. You should have as many tuples as `NUM_LEAF_NODES`. Run this script with one argument **when you are ready to begin the test**. The argument can be literally anything 

Upload `central_node.ino` to the central node and `leaf_node.ino` to the leaf node

Pray

## Testing Checkpoints

1. CN Pi can talk to Arduino
2. LN Arduino can "interrupt" Pi
3. LoRa messages are sent and received (with echo check)
4. Just go with the flo yo