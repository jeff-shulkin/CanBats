#include <LoRa.h>
//#include "LoRa.h"
#include <SPI.h>

#define NODE_ID 0
#define PI_INTERRUPT 4

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1

static bool toggle_interrupt = 0;
int counter;
void send_pi_command(uint8_t cmd);
void get_pi_data();

void setup() {
    counter = 0;
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    
    if (!LoRa.begin(915E6)) {
        while (1);  // LoRa failed
    }
//    LoRa.beginPacket();
//    LoRa.print("ninechars");
//    LoRa.endPacket();
    //Serial.println("Leaf node");
}

void loop() {
//    int received = LoRa.parsePacket();
//    if (received)
//        lora_receive();
  get_pi_data();
  delay(5000);
}

void send_pi_command(char cmd) {
    // interrupt the pi so it can receive the command
    toggle_interrupt = !toggle_interrupt;
    digitalWrite(PI_INTERRUPT, toggle_interrupt);
    delay(150);     // wait to ensure the pi is ready for receiving

    Serial.print(cmd);
    while (!Serial.available());
    char echo = Serial.read();

    if (echo != cmd) {
        // Handle the error somehow
        Serial.print("error");
        while (1);
    }
}

void get_pi_data() {
    send_pi_command((char)DATA_REQUEST);

    uint8_t buff[9];

    while (1) {
        Serial.print((char)0);    // send ready
        
        // read 9 bytes of data at a time
        //digitalWrite(LED_BUILTIN, HIGH);
        while (Serial.available() < 9);
        //digitalWrite(LED_BUILTIN, LOW);
        for (uint8_t i=0; i<9; ++i) {
            buff[i] = Serial.read();
        }
//        for(int i=0; i<9; ++i) {
//          char fstr[10];
//          sprintf(fstr, "%#04x, ", buff[i]);
//          Serial.print(fstr);
//        }
//        Serial.println();
/* Commented out by Tyler (11/15/2023) to test leaf to central communication
 *      // wait for central node ready
        int ready = 0;
        while (!ready)
            ready = LoRa.parsePacket();
        uint8_t junk = LoRa.read(); // discard ready byte
*/
        if (buff[4] == 0xFF)    // PI sent stopcode, done transmitting
            break;
            
        // Send 9 byte packet to central node
        LoRa.beginPacket();
//        for(int j = 0; j < 9; j++){
//          LoRa.print(buff[j]);
//        }
        LoRa.write(buff,9);
//        LoRa.print("This is packet: ");
//        LoRa.print(++counter);
        LoRa.endPacket();
        delay(100);

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
