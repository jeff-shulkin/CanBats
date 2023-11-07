#include <Arduino_FreeRTOS.h>
#include <semaphr.h>
#include <Wire.h>
#include <SPI.h>


// GPIO Pin Definitions
#define LORA_RECEIVE_INT 13 // TODO : MAKE SURE LORA RECEIVE INTERRUPT IS ENABLED AND UPDATE WITH RIGHT PIN
#define RPI_POWER_ENABLE 25
#define MPPT_CHARGE_ENABLE 26

// UART Serial Definitions
#define RPI_BAUD_RATE 115200

// BMS Definitions
#define BMS_ADDR 0x55
#define MAC_SUBCMD 0x3E
#define MAC_MFG_STATUS 0x57
#define GUAGING 0x21
#define PARAMS_ADDR 0x4685

// Task Declarations
void rpi_enable(void *pvParameters); // turns on RPI power
void mppt_enable(void *pvParameters); // turns on MPPT charging
void poll_battery(void *pvParameters); // polls and sends battery level over LoRa

void send_pi_msg(const char[] msg); // send message over UART to RPI 
void receive_pi_msg(const char[] msg); // receive message over UART from RPI
void transmit_lora_msg(const char[] msg); // send message out over LoRa back to gateway
void receive_lora_msg(const char[] msg); // recevie message over LoRa from gateway
void write_bms_df(uint8_t addr, const uint8_t[] data, uint8_t size); // write data to BMS Data Flash

// Semaphore definitions
SemaphoreHandle_t uart_mutex; 
SemaphoreHandle_t i2c_mutex;
SemaphoreHandle_t spi_mutex;

SemaphoreHandle_t lora_receive;

// Function Declarations
void bms_setup(void); // initialize the BMS chip at startup
int parse_msg(const char[] msg); // parses a LoRa packet
void receive_lora_ISR(void); // ISR called on rising edge of LoRa interrupt

void setup() {
  // put your setup code here, to run once:
  // GPIO setup
  pinMode(RPI_POWER_ENABLE, OUTPUT);
  pinMode(MPPT_CHARGE_ENABLE, OUTPUT);

  // SPI setup
  SPISettings LoraSettings(speedMaximum, MSBFIRST, SPI_MODE0); // TODO : VALUES ARE STANDINS RIGHT NOW: REPLACE WITH REAL SETTINGS
  SPI.begin();

  // I2C setup
  Wire.begin();

  // UART setup
  Serial.begin(RPI_BAUD_RATE);

  // FreeRTOS Task Creation
  // TODO : Change Priorities to appropriate levels
  xTaskCreate(
    rpi_enable
    ,  "rpi_enable"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    mppt_enable
    ,  "mppt_enable"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    poll_battery
    ,  "poll_battery"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    send_pi_msg
    ,  "send_pi_msg"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    receive_pi_msg
    ,  "receive_pi_msg"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    transmit_lora_msg
    ,  "transmit_lora_msg"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    receive_lora_msg
    ,  "receive_lora_msg"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
  // FreeRTOS Semaphore/Mutex Creation
  uart_mutex = xSemaphoreCreateMutex();
  i2c_mutex = xSemaphoreCreateMutex();
  spi_mutex = xSemaphoreCreateMutex();

  receive_lora = xSemaphoreCreateBinary();
  // Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(LORA_RECEIVE_INT), receive_lora_msg, RISING);
}

// Task definitions
void rpi_enable(void *pvParameters) {
  void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(enable_pi, xLastWakeTime)) {
      digitalWrite(RPI_POWER_ENABLE, HIGH);
    }
  }
}

void mppt_enable(void *pvParameters) {

}

void poll_battery(void *pvParameters) {
  void(pvParameters);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
      xSemaphoreTake(i2c_mutex); // Begin critical section
      //... TODO : FILL OUT I2C COMMANDS TO GET BATTERY DATA
      xSemaphoreGive(i2c_mutex); // End critical section
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250000)); // poll the battery every 5 minutes

  }
}

void receive_lora_msg(void *pvParameters) {
  (void) pvParameters;
  TickType_t xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();
}
// Non-Task Function defintions

void bms_setup() {
  // Request ManufacturingStatus()
  Wire.beginTransmission(BMS_ADDR);
  Wire.write(MFG_STATUS);
  Wire.write(0);
  Wire.endTransmission();

  // Get ManufacturingStatus()
  Wire.requestFrom(BMS_ADDR, 6);
  while (Wire.available() < 6);
  uint16_t cmd_check = Wire.read() + (Wire.read() << 8);
  assert(cmd_check == MFG_STATUS);  //check that the correct command was sent
  uint16_t manufacturing_status = Wire.read() + (Wire.read() << 8);
  Wire.read();  // NOTE: STILL NEED TO CHECK THE CHECKSUM
  assert(Wire.read() == 6); //check that length == 6

  if (!(manufacturing_status & (1 << 3))) { // if gauging is not enabled, enable it
    Wire.beginTransmission(BMS_ADDR);
    Wire.write(BMS_ADDR);
    Wire.write(GAUGING);
    Wire.write(0);
    Wire.endTransmission();
  }

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

  Wire.endTransmission;
}

int parse_packet() {
  // TODO : Define and complete script for packet parsing
}

void recevie_lora_ISR(void) {
  // On Rising edge of RX_Complete interrupt, unblock "receive_lora_msg"
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(recieve_lora, xHigherPriorityTaskWoken);
}

void write_bms_df(uint16_t addr, const uint8_t[] data, uint8_t size) {
  

  for (uint8_t i=0; i<size; ++i)
    Wire.write(data[i]);
  
  Wire.endTransmission();
}

void loop() {
  // put your main code here, to run repeatedly:

}
