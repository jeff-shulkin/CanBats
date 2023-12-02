#include <LoRa.h>
//#include "LoRa.h"
#include <SPI.h>

#define NODE_ID 0
#define PI_INTERRUPT 4

#define DATA_REQUEST 0
#define SERVER_UPLOAD 0x01
#define STOPCODE 0xFF

static bool toggle_interrupt = 0;
int counter;
void onReceive(int packetSize);
uint8_t leaf_id;

void setup() {
    counter = 0;
    //pinMode(7, OUTPUT);
    Serial.begin(9600);
   
    if (!LoRa.begin(915E6)) {
        //digitalWrite(7,HIGH);
        while (1);  // LoRa failed
    }
    LoRa.setPins(10,2,3);
    LoRa.onReceive(onReceive);
    LoRa.receive();

    uint16_t timeout = 0;
    // wait until RPi boots up
    while (timeout < 300 && Serial.available() < 1) {
      delay(5000);
      timeout += 5;
    }
}

void loop() {
  if(counter > 0){
    Serial.print(SERVER_UPLOAD);
  }
  //delay(300000);
}

void onReceive(int packetSize) {
  if (packetSize==1) {
    leaf_id = LoRa.read();
    if(leaf_id == STOPCODE){
      Serial.print((char)leaf_id);
      ++counter;
    }
    return;
  }
  Serial.print((char)leaf_id);
  for (uint8_t i=0; i<9; ++i)
    Serial.print((char)LoRa.read());
}
