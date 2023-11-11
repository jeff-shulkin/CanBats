
#include "LoRa.h"
#include <SPI.h>

#define NODE_ID 0
#define PI_INTERRUPT 25

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1

static bool toggle_interrupt = 0;

void send_pi_command(uint8_t cmd) {
    // interrupt the pi so it can receive the command
    digitalWrite(PI_INTERRUPT, toggle_interrupt);
    toggle_interrupt = !toggle_interrupt;
    delay(150);     // wait to ensure the pi is ready for receiving

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
        Serial.print(0);    // send ready

        // read 9 bytes of data at a time
        while (Serial.available() < 9);
        for (uint8_t i=0; i<9; ++i) {
            buff[i] = Serial.read();
        }

        // wait for central node ready
        int ready = 0;
        while (!ready)
            ready = LoRa.parsePacket();
        uint8_t junk = LoRa.read(); // discard ready byte

        // Send 9 byte packet to central node
        LoRa.beginPacket();
        LoRa.write(buff, 9);
        LoRa.endPacket();

        if (buff[4] == 0xFF)    // PI sent stopcode, done transmitting
            break;
    }

}

void lora_receive() {
    uint8_t node = LoRa.read();
    uint8_t cmd = LoRa.read();

    if (node != NODE_ID)
        return;

    LoRa.beginPacket();
    LoRa.write(NODE_ID);
    LoRa.write(cmd);
    LoRa.endPacket();

    switch (cmd) {
        case DATA_REQUEST:
            get_pi_data();
            break;
    }
}