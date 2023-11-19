#include <Wire.h>

// BMS Definitions
#define BMS_ADDR 0x55
#define MAC_SUBCMD 0x3E
#define MAC_MFG_STATUS 0x57
#define GAUGING 0x21
#define PARAMS_ADDR 0x4685
#define MAC_BUFF_SIZE (32)

void setup() {
    Serial.begin(9600);

    uint8_t data_buff[MAC_BUFF_SIZE];
    mac_read_command(data_buff, MAC_BUFF_SIZE, MAC_MFG_STATUS);
    uint16_t mfg_status = data_buff[0] | (data_buff[1]<<8);
    
    uint8_t gauging_en = (mfg_status>>3) & 1;
    
    if (gauging_en) {
        Serial.println("Gauging is already enabled, setup is done");
    }
    else {
        Serial.println("Gauging is not enabled, sending write instruction");

        toggle_gauging();
    }
}

void toggle_gauging() {
    Wire.beginTransmission(BMS_ADDR);
    Wire.write(MAC_SUBCMD); // MAC starting address
    Wire.write(GAUGING);    // Send low byte of gauging cmd
    Wire.write(GAUGING >> 8);   // Send high byte of gauging cmd 
    Wire.endTransmission();
}

void mac_read_command(uint8_t *data, uint8_t data_capacity, uint16_t subcmd) {
  // Request ManufacturingStatus()
  if (data_capacity < MAC_BUFF_SIZE) {
    Serial.println("Data capacity not met");
    return;
  }
  Serial.println("Sending I2C message to get Manufacturing Status");

  Wire.beginTransmission(BMS_ADDR);
  Wire.write(MAC_SUBCMD);   // MAC starting address
  Wire.write(subcmd);   // send low byte of mfg status cmd
  Wire.write(subcmd >> 8);  // send high byte of mfg status cmd
  Wire.endTransmission();

  // Get ManufacturingStatus()
  Wire.requestFrom(BMS_ADDR, MAC_BUFF_SIZE+4);
  while (Wire.available() < MAC_BUFF_SIZE+4) {
    Serial.println("Waiting to receive %d bytes", MAC_BUFF_SIZE+4);
    delay(1000);
  }

  Serial.println("Bytes received");

  uint8_t mac_buff[MAC_BUFF_SIZE+4];

  for (uint8_t i=0; i<MAC_BUFF_SIZE+4; ++i) {
    mac_buff[i] = Wire.read();
  }

  uint16_t cmd_check = mac_buff[0] | (mac_buff[1] << 8);

  char strbuf[50];
  sprintf(strbuf, "The command sent was %#04x, we received %#04x", MAC_MFG_STATUS, cmd_check);
  Serial.println(strbuf);
  
  uint8_t len = mac_buff[35];
  uint8_t checksum = mac_buff[34];

  uint8_t validate = mac_buff[0]+mac_buff[1];
  for(int i = 0; i < len - 4; ++i)
  {
    validate += mac_buff[i+2];
    data[i] = mac_buff[i+2];
  }

  if(checksum != (~validate))
  {
    Serial.println("Check Sum failed for data");
    return;
  }

  // // this is for a two byte command
  // // TODO: abstract code, use length
  // uint16_t manufacturing_status = Wire.read() + (Wire.read() << 8);
  // sprintf(strbuf, "Raw manufacturing_status val: %#04x");
  // Serial.println(strbuf);

  // Wire.read();  // NOTE: STILL NEED TO CHECK THE CHECKSUM

  // uint8_t len = Wire.read();    // the size of the message is also transmitted
  // Serial.print("Expected len 6, got");
  // Serial.println(len);

  // return manufacturing_status;
}

void bms_setup() {

  // charging values
  int16_t dschg_curr_thresh = 0;
  int16_t chg_curr_thresh = 0;
  int16_t quit_curr_thresh = 0;
  uint8_t dschg_relax_time = 0;
  uiint8_t chg_relax_time = 0;

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