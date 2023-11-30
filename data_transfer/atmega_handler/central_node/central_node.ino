#include <LoRa.h>
//#include "LoRa.h"
#include <SPI.h>

#define NODE_ID 0
#define PI_INTERRUPT 4

#define DATA_REQUEST 0
#define GET_LEAF_DATA 1
#define STOPCODE 0xFF

static bool toggle_interrupt = 0;
int counter;
void onReceive(int packetSize);

void setup() {
    counter = 0;
    pinMode(7, OUTPUT);
    Serial.begin(9600);
   
    if (!LoRa.begin(915E6)) {
        //digitalWrite(7,HIGH);
        while (1);  // LoRa failed
    }
    LoRa.onReceive(onReceive);
    LoRa.receive();
}

void loop() {
//    int received = LoRa.parsePacket();
//    if (received)
//        lora_receive();
}

void onReceive(int packetSize) {
  if (packetSize==1) {
    leaf_id = LoRa.read();
    if(leaf_id == STOPCODE){
      Serial.print((char)leaf_id);
    }
    return;
  }
  Serial.print(leaf_id);
  for (uint8_t i=0; i<9; ++i)
    Serial.print((char)LoRa.read());
}