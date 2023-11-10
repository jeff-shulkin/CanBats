
#include "LoRa.h"
#include <SPI.h>

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1

void send_pi_command(uint8_t cmd) {
    Serial.print(cmd);
    while (!Serial.available());
    uint8_t ack = Serial.read();

    if (ack != cmd) {
        // Handle the error somehow
        while (1);
    }
}

void get_pi_data() {
    send_pi_command(DATA_REQUEST);

    char buff[9];

    while (1) {
        // read 9 bytes of data at a time
        for (uint8_t i=0; i<9; ++i) {
            while (!Serial.available());
            buff[i] = Serial.read();
        }

        // Send 9 byte packet to central node
        LoRa.beginPacket();
        LoRa.write(buff, 9);
        LoRa.endPacket();

        if (buff[4] == 0xFF)    //PI sent stopcode, done transmitting
            break;
    }

}

void lora_receive() {
    uin8_t cmd = LoRa.read();

    switch (cmd) {
        case DATA_REQUEST:
            get_pi_data();
            break;
    }
}