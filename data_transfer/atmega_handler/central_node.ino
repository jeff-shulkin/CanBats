
#include "LoRa.h"
#include <SPI.h>

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1

void setup() {
    Serial.begin(9600);

    if (!LoRa.begin(915E6)) {
        while (1);  // LoRa failed
    }
}

void loop() {
    if (Serial.available())
        serial_receive();
}

void send_lora_command(uint8_t node, uint8_t cmd) {
    // assuming LoRa is already running
    LoRa.beginPacket();
    LoRa.write(node);
    LoRa.write(cmd);
    LoRa.endPacket();

    int packetSize = 0;
    // wait for ack
    while (!packetSize)
        packetSize = LoRa.parsePacket();
    uint8_t resp_node = LoRa.read(), resp_cmd = LoRa.read();
    if (resp_node != node || resp_cmd != cmd) {
        // we have an error
        while (1);
    }
}

void serial_receive() {
    uint8_t cmd = Serial.read();
    Serial.print(cmd);  // ack
    switch (cmd) {
        case GET_LEAF_DATA:
            collect_leaf_data(Serial.read());
    }
}

void collect_leaf_data(uint8_t node) {
    send_lora_command(node, DATA_REQUEST);
        
    // Continually get 9 byte packets
    char buff[9];

    int packetSize = 0;
    while (1) {
        LoRa.beginPacket();
        LoRa.write(0);  // send ready
        LoRa.endPacket();
        
        while (!packetSize)    // keep waiting for a packet
            packetSize = LoRa.parsePacket();

        if (packetSize != 9) {
            //idk what to do
            while (1);
        }

        packetSize = 0;

        // wait for pi ready
        while (!Serial.available());

        for (uint8_t i=0; i<9; ++i) {
            buff[i] = LoRa.read();
            Serial.print(buff[i]);
        }

        if (buff[4] == 0xFF)    // stop condition met
            break;
    }

}