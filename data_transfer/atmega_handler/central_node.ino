
#include "LoRa.h"
#include <SPI.h>

#define INTERRUPT_PI 7

bool interrupt;

char leaf_id;

void onReceive(int);

void setup() {
    Serial.begin(9600);

    if (!LoRa.begin(915E6)) {
        while (1);  // LoRa failed
    }

    LoRa.onReceive(onReceive);
    LoRa.receive();

    interrupt = 0;
    
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
}

void loop() {
    
}


//  THIS FUNCTION WAS USED TO TEST DIRECTLY TO PI
// void data_test() {
//   char buff[10];

//   uint32_t time = 1002003004;
//   uint8_t species = 5;
//   float conf = 150.20;

//   memcpy(buff, &time, 4);
//   buff[4] = species;
//   memcpy(buff+5, &conf, 4);

//   Serial.print((char)leaf_id);

//   while (!Serial.available());
//   Serial.read();

//   for (uint8_t i=0; i<9; ++i)
//     Serial.print((buff[i]));
// }

void onReceive(int packetSize) {
  //Serial.println("LoRa message received!");

  if (packetSize==1) {
    //Serial.println("Packet is 1 byte");
    leaf_id = LoRa.read();

    // LoRa.beginPacket();
    // LoRa.write(leaf_id);
    // LoRa.endPacket();
    
    //Serial.println("Sent echo");
  
    return;
  }

  interrupt = !interrupt;
  //digitalWrite(INTERRUPT_PI, interrupt);
  Serial.print(leaf_id);
  while (!Serial.available()) {
    digitalWrite(4, LOW);
    delay(2000);
  }

  Serial.read();

  for (uint8_t i=0; i<9; ++i)
    Serial.print(LoRa.read());
    Serial.println("i");
}