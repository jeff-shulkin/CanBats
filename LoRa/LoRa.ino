#include <LoRa.h>
#include <SPI.h>

// TODO: look Arduino SPI pinout pins 
#define LORA_MISO 12
#define LORA_MOSI 11
#define LORA_SCK  13
#define LORA_NSS  10
#define REG_RSSI_WIDEBAND        0x2c


char buff[] = "Hello";
int buff_size = 5;
uint8_t response = 0xFF;

void setup() {
  // put your setup code here, to run once:
  pinMode(LORA_MISO, INPUT);
  pinMode(LORA_MOSI, OUTPUT);
  pinMode(LORA_SCK, OUTPUT);
  pinMode(LORA_NSS, OUTPUT);

  Serial.begin(9600);

  SPI.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(buff);
  response = 0xFF;
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(LORA_NSS, LOW);
  SPI.transfer(REG_RSSI_WIDEBAND);
  response = SPI.transfer(0x00);
  digitalWrite(LORA_NSS, HIGH);
  SPI.endTransaction();
  Serial.println(response);
  
}
