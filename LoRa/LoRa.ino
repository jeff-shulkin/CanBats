//#include <LoRa.h>


#include "LoRa.h"
#include <SPI.h>

#define LORA_MISO 12
#define LORA_MOSI 11
#define LORA_SCK  13
#define LORA_NSS  10

void test_receive();
void test_send();


char buff[] = "Hello";
int buff_size = 5;
uint8_t response = 0xFF;
int counter = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LORA_MISO, INPUT);
  pinMode(LORA_MOSI, OUTPUT);
  pinMode(LORA_SCK, OUTPUT);
//  pinMode(LORA_NSS, OUTPUT);
  
  Serial.begin(9600);

  Serial.println("LoRa Receiver");
  //LoRa.begin(915E6);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  //SPI.begin();

  
}

void loop() {
  test_receive();
  test_send();
}

void test_receive(){
    // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}

void test_send(){
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
