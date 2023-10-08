#include <SPI.h>

// TODO: look Arduino SPI pinout pins 
#define LORA_MISO 1
#define LORA_MOSI 2
#define LORA_SCK  3
#define LORA_NSS  4

char buff[50];
int buff_size = 50;

void setup() {
  // put your setup code here, to run once:
  pinMode(LORA_MISO, INPUT);
  pinMode(LORA_MOSI, OUTPUT);
  pinMode(LORA_SCK, OUTPUT);
  pinMode(LORA_NSS, OUTPUT);

  SPI.begin();
  buff[0] = 'H';
  buff[1] = 'e';
  buff[2] = 'l';
  buff[3] = 'l';
  buff[4] = 'o';
}

void loop() {
  // put your main code here, to run repeatedly:
  SPI.beginTransaction(SPISettings(915000000, MSBFIRST, SPI_MODE0));
  SPI.tranfser(buff, buff_size);
  SPI.endTransaction();
  
}
