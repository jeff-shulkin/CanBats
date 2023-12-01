#include <Wire.h>
#include <LoRa.h>

// BMS Definitions
#define BMS_ADDR 0x55
#define MAC_SUBCMD 0x3E
#define MFG_STATUS 0x57
#define GAUGING 0x21
#define PARAMS_ADDR 0x4685
#define MAC_BUFF_SIZE (36)//MAKE SURE TO FIX THIS WAS 32 ORIGINALLY
#define RSOC 0x2C
#define BATT_STAT 0x0A 

void setup() {
    Serial.begin(9600);
    Serial.println("Starting in setup...");
    
    if (!LoRa.begin(915E6)) {
      Serial.println("Starting LoRa failed!");
      while (1);
    }
    
    Wire.begin();

    //Set default battery charge
    Serial.println("Sending Battery Design Capacity.");
    Wire.beginTransmission(BMS_ADDR);
    Wire.write(0x29);
    Wire.write(0x46);
    Wire.write(0x50);
    Wire.write(0x14);
    Serial.println("Design Capacity sent");
    Wire.endTransmission();

    Serial.println("Requesting manufacturing status");
    uint8_t data_buff[MAC_BUFF_SIZE];
    int ret = mac_read_command(data_buff, MAC_BUFF_SIZE, MFG_STATUS);
    uint16_t mfg_status = data_buff[0] | (data_buff[1]<<8);
    char strbuf[50];
    sprintf(strbuf, "Received data: %#06x", mfg_status);
    Serial.println(strbuf);
    
    uint8_t gauging_en = (mfg_status>>3) & 1;
    
    if (gauging_en) {
        Serial.println("Gauging is already enabled, setup is done");
    }
    else {
        Serial.println("Gauging is not enabled, sending write instruction");
        toggle_gauging();
    }

    //Serial.print("RSOC = ");
    //Serial.println(read_RSOC());    
}

uint8_t read_RSOC() {
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(RSOC);
  Wire.endTransmission();

  Wire.requestFrom(BMS_ADDR, 1);
  while(Wire.available() < 1);
  return Wire.read();
}

void toggle_gauging() {
    Wire.beginTransmission(BMS_ADDR);
    Wire.write(MAC_SUBCMD); // MAC starting address
    Wire.write(GAUGING);    // Send low byte of gauging cmd
    Wire.write(GAUGING >> 8);   // Send high byte of gauging cmd 
    Wire.endTransmission();
}

int mac_read_command(uint8_t *data, uint8_t data_capacity, uint16_t subcmd) {
  // Request ManufacturingStatus()
  if (data_capacity < MAC_BUFF_SIZE) {
    Serial.println("Data capacity not met");
    return -1;
  }

  Wire.beginTransmission(BMS_ADDR);
  Serial.println("transmission begun");
  Wire.write(MAC_SUBCMD);   // MAC starting address
  Wire.write(subcmd);
  Wire.write(subcmd >> 8);
  int ret = Wire.endTransmission();
  Serial.println(ret);
  Serial.println("I2C message sent, reading from I2C");

  // Reset start address and perform the read
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(MAC_SUBCMD);
  Wire.endTransmission();

  Wire.requestFrom(BMS_ADDR, 18);

  uint8_t mac_buff[36];

  for (uint8_t i=0; i<18; ++i) {
    while(Wire.available() < 1);
    mac_buff[i] = Wire.read();
    Serial.print(i);
    Serial.print(' ');
  }

  //second transaction
  Wire.requestFrom(BMS_ADDR, 18);

  for (uint8_t i=18; i<36; ++i) {
    while(Wire.available() < 1);
    mac_buff[i] = Wire.read();
    Serial.print(i);
    Serial.print(' ');
  }

  Serial.println("\nBytes received");

  uint16_t cmd_check = mac_buff[0] | (mac_buff[1] << 8);

  char strbuf[50];
  sprintf(strbuf, "The command sent was %#04x, we received %#04x", subcmd, cmd_check);
  Serial.println(strbuf);
  
  uint8_t len = mac_buff[35];
  uint8_t checksum = mac_buff[34];

  uint8_t check_checksum = mac_buff[0]+mac_buff[1];
  for(int i = 0; i < len - 4; ++i)
  {
    check_checksum += mac_buff[i+2];
    data[i] = mac_buff[i+2];
  }

  check_checksum = ~check_checksum;

  sprintf(strbuf, "Caculated checksum %#04x, we received %#04x", check_checksum, checksum);
  Serial.println(strbuf);
  if(checksum != check_checksum)
  {
    Serial.println("Check Sum failed for data");
    return -1;
  }

  return len;
}

void bms_setup() {

  // charging values
  int16_t dschg_curr_thresh = 0;
  int16_t chg_curr_thresh = 0;
  int16_t quit_curr_thresh = 0;
  uint8_t dschg_relax_time = 0;
  uint8_t chg_relax_time = 0;

  // write all these values to data flash in little endian
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(MAC_SUBCMD);
  Wire.write(PARAMS_ADDR);
  Wire.write(PARAMS_ADDR >> 8);

  Wire.write(dschg_curr_thresh);
  Wire.write(dschg_curr_thresh >> 8);
  Wire.write(chg_curr_thresh);
  Wire.write(chg_curr_thresh >> 8);
  Wire.write(quit_curr_thresh);
  Wire.write(quit_curr_thresh >> 8);
  Wire.write(dschg_relax_time);
  Wire.write(chg_relax_time);

  Wire.endTransmission();
}

void do_it_be_charging() {
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(BATT_STAT);
  Wire.endTransmission();

  Wire.requestFrom(BMS_ADDR, 1);
  while(Wire.available() < 1) {
    delay(10);
  }
  LoRa.println("Got battery status");
  uint8_t status = Wire.read();
  uint8_t error_code = status & ~0b111;
  LoRa.println(error_code ? "No error code" : "Got an error :(");
  uint8_t depleted = status & ~(1<<4);
  LoRa.println(depleted ? "Battery is dead" : "Battery is alive");
  uint8_t full = status & ~(1<<5);
  LoRa.println(full ? "Battery is fully charged" : "Battery is not fully charged");
  uint8_t charging = status & ~(1<<6);
  LoRa.println(charging ? "Battery is discharging" : "Battery is charging");
}

void loop() {
  do_it_be_charging();
  delay(10000);
}