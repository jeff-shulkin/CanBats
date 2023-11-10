
#include "LoRa.h"
#include <SPI.h>

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1

void send_lora_command(uint8_t node, uint8_t cmd) {
    // assuming LoRa is already running
    LoRa.beginPacket();
    LoRa.print(node);
    LoRa.print(cmd);
    LoRa.endPacket();

    int packetSize = 0;
    // wait for ack
    while (!packetSize)
        packetSize = LoRa.parsePacket();
    if ((uint8_t)LoRa.read() != node) {
        // we have an error
        while (1);
    }
}

void serial_receive() {
    uint8_t cmd = Serial.read();
    Serial.print(cmd);  // ack
    switch (cmd) {
        case GET_LEAF_DATA:
            send_leaf_data(Serial.read());
    }
}

void send_leaf_data(uint8_t node) {
    send_lora_command(node, DATA_REQUEST);
        
    // Continually get 9 byte packets
    char buff[9];

    while (1) {
        int packetSize = LoRa.parsePacket();
        if (!packetSize)
            continue;

        if (packetSize != 9) {
            //idk what to do
        }

        for (uint8_t i=0; i<9; ++i) {
            buff[i] = LoRa.read();
            Serial.print(buff[i]);
        }

        if (buff[4] == 0xFF)    // stop condition met
            break;
    }

}