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

### PI to PI
- `'SEND_DATA'`: send the recently collected bat data to the ATmega for transmission
- `'GET_LEAF_DATA'`: get all leaf data, stores data in csv file

### PI to ATmega
- `GET_LEAF_DATA (0x00)`: Sends ATmega a leaf node ID and requests that the ATmega sends the PI the data from that leaf node

### ATmega to PI
- `DATA_REQUEST (0x00)`: request the the recently collected bat data

### ATmega to ATmega (LoRa)
LoRa commands are two bytes. The first is the target node ID and the second is the command. Upon receiving, the target node first transmits its own ID as acknowledgement
- `DATA_REQUEST (0x00)`: requests that the receiving node sends its recently collected bat data