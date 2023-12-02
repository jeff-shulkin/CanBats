#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <avr/sleep.h> 
#include <Wire.h>
#include <LoRa.h>
//#include <SPI.h>

// NODE ID
// Different for each node
// TODO: change each node's id to unique
#define NODE_ID 0x00

// GPIO Pin Definitions
//#define LORA_RECEIVE_INT 13 // TODO : MAKE SURE LORA RECEIVE INTERRUPT IS ENABLED AND UPDATE WITH RIGHT PIN
//#define RPI_POWER_ENABLE 25
//#define MPPT_CHARGE_ENABLE 26

// UART Serial Definitions
#define RPI_BAUD 9600

// RPi Communication Definitions
#define SEND_LEAF_DATA 0x00
#define START_AI 0x01
#define STOP_AI 0x02
#define START_RECORDING 0x03
#define STOP_RECORDING 0x04
#define GET_TIME 0x05
#define STOPCODE 0xFF

// BMS Definitions
// For a lot of address, they use a R/W address convention as follows:
// For given ADDR_EXAMPLE = 0xAA/0xAB: 0xAA == Write, 0xAB == Read
#define BMS_ADDR 0x55
#define MAC_SUBCMD 0x3E
#define MAC_MFG_STATUS 0x57
#define MFG_STATUS 0x57
#define MAC_BUFF_SIZE 32
#define GAUGING 0x21
#define RSOC_READ 0x2D
#define PARAMS_ADDR 0x4685

// Task Declarations

void poll_battery(void *pvParameters); // polls battery level
void ai_processing(void *pvParameters);
void recording(void *pvParameters);
void send_leaf_data(void *pvParameters);
void timer(void *pvParameters);

// Function Declarations

void rpi_enable(); // turns on RPI power
void mppt_enable(); // turns on MPPT charging
//void send_pi_msg(const char[] msg); // send message over UART to RPI 
//void transmit_lora_msg(const char[] msg); // send message out over LoRa back to gateway
//void receive_lora_msg(const char[] msg); // recevie message over LoRa from gateway
bool mac_read_command(uint8_t *data, uint8_t data_capacity, uint16_t subcmd); // perform a MAC rea on BM
//void write_bms_df(uint8_t addr, const uint8_t[] data, uint8_t size); // write data to BMS Data Flash

//void bms_setup(void); // initialize the BMS chip at startup
//int parse_msg(const char[] msg); // parses a LoRa packet
//void receive_lora_ISR(void); // ISR called on rising edge of LoRa interrupt

uint8_t read_RSOC();
void toggle_gauging();

// Global Booleans
bool is_recording = false;
bool is_processing = false;
uint8_t battery_rsoc = 0;

// Global ints
uint32_t _time;

SemaphoreHandle_t processingMutex = NULL;
SemaphoreHandle_t sendMutex = NULL;

void setup() {

  // put your setup code here, to run once:
  // GPIO setup
  pinMode(8, OUTPUT);
  rpi_enable();

  pinMode(A3, OUTPUT);
  mppt_enable();

  if(!LoRa.begin(915E6)){
      while(1);
  }

  // I2C setup
  Wire.begin();
  //Serial.println("Entering Default battery Charge.");
  //Serial.println("Sending Slave addr.");
  Wire.beginTransmission(BMS_ADDR);
  //Serial.println("Writing data");
  Wire.write(0x29);
  Wire.write(0x48);
  Wire.write(0x50);
  Wire.write(0x14);
  //Serial.println("Ending transmission");
  Wire.endTransmission();

  uint8_t data_buff[MAC_BUFF_SIZE];
  int ret = mac_read_command(data_buff, MAC_BUFF_SIZE, MFG_STATUS);
  uint16_t mfg_status = data_buff[0] | (data_buff[1]<<8);

  uint8_t gauging_en = (mfg_status>>3) & 1;
    
  if (!gauging_en) {
      toggle_gauging();
  }

  // UART setup
  Serial.begin(RPI_BAUD);
  
  uint16_t timeout = 0;
  // wait until RPi boots up
  while (timeout < 300 && Serial.available() < 1) {
    delay(5000);
    timeout += 5;
  }

  _time = 0;
  Serial.print((char)GET_TIME);
  while (Serial.available() < 4);
  for (uint8_t i=0; i<32; i += 8) {
    _time |= Serial.read() << i;
  }

  // FreeRTOS Task Creation
  // TODO : Change Priorities to appropriate levels

  xTaskCreate(
    poll_battery
    ,  "poll_battery"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    ai_processing
    ,  "ai_processing"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );


  xTaskCreate(
    recording
    ,  "recording"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
  xTaskCreate(
    send_leaf_data
    ,  "send_leaf_data"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
  xTaskCreate(
    timer
    ,  "timer"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
  processingMutex = xSemaphoreCreateMutex();
  sendMutex = xSemaphoreCreateMutex();
}

// Task definitions

// TODO: Write this function
void poll_battery(void *pvParameters) {
  //void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
      //xSemaphoreTake(i2c_mutex); // Begin critical section
      //... TODO : FILL OUT I2C COMMANDS TO GET BATTERY DATA
      battery_rsoc = read_RSOC();
      if(battery_rsoc < 10){
        digitalWrite(RPI_POWER_ENABLE, LOW);
      }else if(battery_rsoc > 80){
        //xSemaphoreGive(processingMutex);
      }
      //xSemaphoreGive(i2c_mutex); // End critical section
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(600000)); // poll the battery every 10 minutes

  }
}

void ai_processing(void *pvParameters){
  //void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    //xSemaphoreTake(processingMutex);
    if (is_processing) {
      is_processing = false;
      Serial.print(STOP_AI);
      xSemaphoreGive(sendMutex);
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(82800000));
    } else {
      is_processing = true;
      Serial.print(START_AI);
      vTaskDelay(pdMS_TO_TICKS(3600000)); // 1 hour to process
    }
  }
} // ai_processing() Task

void recording(void *pvParameters){
  //void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
    if (is_recording) {
      is_recording = false;
      Serial.print(STOP_RECORDING);
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(600000));
    } else {
      is_recording = true;
      Serial.print(START_RECORDING);
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
} // recording() Task

void send_leaf_data(void *pvParameters){
  //void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while (1) {
      xSemaphoreTake(sendMutex, portMAX_DELAY);
      // Say which node is sending
      if(NODE_ID == 0x01){
        vTaskDelay(pdMS_TO_TICKS(300000)); // give other node time to upload
      }
      LoRa.beginPacket();
      LoRa.write(NODE_ID);
      LoRa.endPacket();

      // Get data from RPi
      Serial.print(SEND_LEAF_DATA);
      
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
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(86400000)); // every day
  }
} // send_leaf_data Task

void timer(void *pvParameters){
  //void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    _time++;
    if(_time > 86400){
      _time = 0;
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}

// Non-Task Function defintions
void rpi_enable() {
  digitalWrite(8, HIGH);
}

void mppt_enable() {
  digitalWrite(A3, LOW);
}


//void bms_setup() {
//  // Request ManufacturingStatus()
//  uint8_t data_buff[MAC_BUFF_SIZE];
//  bool successful_read = false;
//  while (!successful_read) {
//    successful_read = mac_read_command(data_buff, MAC_BUFF_SIZE, MAC_MFG_STATUS);
//  }
//
//  uint16_t mfg_status = data_buff[0] | (data_buff[1]<<8);
//  
//  uint8_t gauging_en = (mfg_status>>3) & 1;
//  
//  if (!gauging_en) {
//    toggle_gauging();
//  }
//
//  int16_t dschg_curr_thresh = 0;
//  int16_t chg_curr_thresh = 0;
//  int16_t quit_curr_thresh = 0;
//  uint8_t dschg_relax_time = 0;
//  uint8_t chg_relax_time = 0;
//
//  uint8_t dflash_values[8];   // 8 byte array of data to write
//  memcpy(dflash_values, *dschg_curr_thresh, 2);
//  memcpy(dflash_values + 2, *chg_curr_thresh, 2);
//  memcpy(dflash_values + 4, *quit_curr_thresh, 2);
//  dflash_values[6] = dschg_relax_time;
//  dflash_values[7] = chg_relax_time;
//
//  write_bms_df(PARAMS_ADDR, dflash_values, 8);
//}

//void write_bms_df(uint16_t addr, const uint8_t[] data, uint8_t size) {
//  for (uint8_t i=0; i<size/4; ++i) {
//    Wire.beginTransmission(BMS_ADDR);
//    Wire.write(addr);
//    Wire.write(addr >> 8);
//    for (uint8_t j=i*4; j<i*4+4; ++j) {
//      Wire.write(data[j]);
//    }
//    Wire.endTransmission();
//
//    delay(5);
//  }
//}

bool mac_read_command(uint8_t *data, uint8_t data_capacity, uint16_t subcmd) {
  // Request ManufacturingStatus()
  if (data_capacity < MAC_BUFF_SIZE) {
    //Serial.println("Data capacity not met");
    return -1;
  }

  Wire.beginTransmission(BMS_ADDR);
  //Serial.println("transmission begun");
  Wire.write(MAC_SUBCMD);   // MAC starting address
  Wire.write(subcmd);
  Wire.write(subcmd >> 8);
  int ret = Wire.endTransmission();
  //Serial.println(ret);
  //Serial.println("I2C message sent, reading from I2C");

  // Reset start address and perform the read
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(MAC_SUBCMD);
  Wire.endTransmission();

  Wire.requestFrom(BMS_ADDR, 18);

  uint8_t mac_buff[36];

  for (uint8_t i=0; i<18; ++i) {
    while(Wire.available() < 1);
    mac_buff[i] = Wire.read();
    //Serial.print(i);
    //Serial.print(' ');
  }

  //second transaction
  Wire.requestFrom(BMS_ADDR, 18);

  for (uint8_t i=18; i<36; ++i) {
    while(Wire.available() < 1);
    mac_buff[i] = Wire.read();
    //Serial.print(i);
    //Serial.print(' ');
  }

  //Serial.println("\nBytes received");

  uint16_t cmd_check = mac_buff[0] | (mac_buff[1] << 8);

  //char strbuf[50];
  //sprintf(strbuf, "The command sent was %#04x, we received %#04x", subcmd, cmd_check);
  //Serial.println(strbuf);
  
  uint8_t len = mac_buff[35];
  uint8_t checksum = mac_buff[34];

  uint8_t check_checksum = mac_buff[0]+mac_buff[1];
  for(int i = 0; i < len - 4; ++i)
  {
    check_checksum += mac_buff[i+2];
    data[i] = mac_buff[i+2];
  }

  check_checksum = ~check_checksum;

  //sprintf(strbuf, "Caculated checksum %#04x, we received %#04x", check_checksum, checksum);
  //Serial.println(strbuf);
  if(checksum != check_checksum)
  {
    //Serial.println("Check Sum failed for data");
    return -1;
  }

  return len;
}

uint8_t read_RSOC() {
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(RSOC_READ);
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

// This is the Idle Task
// This is what Scheduler runs when there is nothing to run.
// In our case, his will be FREQUENTLY at night, and for large chunks of the day
void loop() {
  // There are several macros provided in the  header file to actually put
  // the device into sleep mode.
  // See ATmega328p Datasheet for more detailed descriptions.
   
  // SLEEP_MODE_IDLE
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_DOWN
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_EXT_STANDBY
   
  set_sleep_mode( SLEEP_MODE_STANDBY );
   
  portENTER_CRITICAL();
  
  sleep_enable();
   
  // Only if there is support to disable the brown-out detection.
  // If the brown-out is not set, it doesn't cost much to check.
  #if defined(BODS) && defined(BODSE)
    sleep_bod_disable();
  #endif
   
  portEXIT_CRITICAL();
  
  sleep_cpu(); // Good night.
   
  // Ugh. Yawn... I've been woken up. Better disable sleep mode.
  // Reset the sleep_mode() faster than sleep_disable();
  sleep_reset();
  
    

}
