#include <LoRa.h>
//#include "LoRa.h"
#include <SPI.h>

#define NODE_ID 0
#define PI_INTERRUPT 4

#define DATA_REQUEST 0x00
//#define SEND_LEAF_DATA 0x00
#define START_AI 0x01
#define STOP_AI 0x02
#define START_RECORDING 0x03
#define STOP_RECORDING 0x04
#define STOPCODE 0xFF

static bool toggle_interrupt = 0;
int counter;
void send_pi_command(uint8_t cmd);
void get_pi_data();

void setup() {
  counter = 0;
  //pinMode(7, OUTPUT);
  Serial.begin(9600);
   
    if (!LoRa.begin(915E6)) {
        //digitalWrite(7,HIGH);
        while (1);  // LoRa failed
    }
  uint16_t timeout = 0;
  // wait until RPi boots up
  while (timeout < 300 && Serial.available() < 1) {
    delay(5000);
    timeout += 5;
  }
}

void loop() {
  //digitalWrite(7, HIGH);
  //AI
  Serial.print(START_AI);
  delay(180000);
  Serial.print(STOP_AI);
  delay(100);
  //SEND LEAF DATA
  Serial.print(DATA_REQUEST);
  LoRa.beginPacket();
  LoRa.write(NODE_ID);
  LoRa.endPacket();
  //digitalWrite(LED_BUILTIN, LOW);
  get_pi_data();
  delay(20000);
}

void send_pi_command(char cmd) {
    // interrupt the pi so it can receive the command
    toggle_interrupt = !toggle_interrupt;
    digitalWrite(PI_INTERRUPT, toggle_interrupt);
    delay(150);     // wait to ensure the pi is ready for receiving

    Serial.print(cmd);
}

void get_pi_data() {
    //digitalWrite(7, LOW);
    //send_pi_command((char)DATA_REQUEST);

    uint8_t buff[9];

    while (1) {
        //Serial.print((char)0);    // send ready
       
        // read 9 bytes of data at a time
        //digitalWrite(LED_BUILTIN, HIGH);
        while (Serial.available() < 9);
        //digitalWrite(LED_BUILTIN, LOW);
        for (uint8_t i=0; i<9; ++i) {
            buff[i] = Serial.read();
        }
        if (buff[4] == STOPCODE) {   // PI sent stopcode, done transmitting
            LoRa.beginPacket();
            LoRa.write(STOPCODE);
            LoRa.endPacket();
            break;
        }
           
        // Send 9 byte packet to central node
        LoRa.beginPacket();
        LoRa.write(buff,9);
        LoRa.endPacket();
        delay(100);
    }
}//get_pi_data()
