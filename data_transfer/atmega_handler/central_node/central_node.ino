#include <LoRa.h>
//#include "LoRa.h"
#include <SPI.h>

#define INTERRUPT_PI 7
#define STOPCODE 0xFF

bool interrupt;

uint8_t leaf_id;

void onReceive(int);

void setup() {
    Serial.begin(9600);
    pinMode(4, OUTPUT);
    if (!LoRa.begin(915E6)) {
        while (1);  // LoRa failed
    }

    LoRa.onReceive(onReceive);
    LoRa.receive();

    interrupt = 0;
    digitalWrite(4, HIGH);
    
    
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
  if (packetSize==1) {
    leaf_id = LoRa.read();
    if(leaf_id == STOPCODE){
      Serial.print((char)leaf_id);
    }
    digitalWrite(4,LOW);
    return;
  }

  interrupt = !interrupt;
  //digitalWrite(INTERRUPT_PI, interrupt);
  digitalWrite(4, HIGH);
  Serial.print(leaf_id);
  //digitalWrite(4, HIGH);
  for (uint8_t i=0; i<9; ++i)
    Serial.print((char)LoRa.read());
}
