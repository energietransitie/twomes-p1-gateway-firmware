#ifndef _P1CONFIG_H
#define _P1CONFIG_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include <driver/uart.h>
#include <driver/gpio.h>



//UART defines
#define P1_BUFFER_SIZE 1024
#define P1PORT_UART_NUM UART_NUM_2

//DeviceTypes
#define DEVICETYPE_P1_ONLY "DSMR-P1-gateway"
#define DEVICETYPE_P1_WITH_SENSORS "DSMR-P1-gateway-TinTsTr"

//HTTP and JSON
#define OFFICIAL_SERVER "https://api.tst.energietransitiewindesheim.nl"
#define OFFICIAL_SERVER_DEVICE_ACTIVATION "https://api.tst.energietransitiewindesheim.nl/device/activate"
#define ACTIVATE_ADDRESS "/device/activate"
#define FIXED_INTERVAL_ADDRESS "/device/measurements/fixed-interval"
#define VARIABLE_INTERVAL_ADDRESS "/device/measurements/variable-interval"
#define JSON_BUFFER_SIZE 4096

//Pin definitions:
#define BUTTON_P1 GPIO_NUM_0
#define BUTTON_P2 GPIO_NUM_12
#define LED_ERROR GPIO_NUM_13
#define LED_STATUS GPIO_NUM_14
#define PIN_DRQ GPIO_NUM_17
#define OUTPUT_BITMASK ((1ULL << LED_ERROR) | (1ULL << LED_STATUS) | (1ULL << PIN_DRQ))
#define INPUT_BITMASK ((1ULL << BUTTON_P1) | (1ULL << BUTTON_P2))

#define MAX_SAMPLES_ESPNOW 60
//Types of measurements that can be received through ESP-Now:
enum ESPNOWdataTypes
{
    BOILERTEMP,
    ROOMTEMP,
    CO2,
};

//Struct should be exact same as in Measurement device, measurement type is enumerated in ESPNOWdataTypes
typedef struct ESP_message
{
    uint8_t measurementType;      //Type of measurement
    uint8_t numberofMeasurements; //number of measurements
    uint16_t index;               //Number identifying the message, only increments on receiving an ACK from Gateway
    uint16_t intervalTime;        //Interval between measurements, for timestamping
    uint8_t data[240];
} ESP_message;

//Struct for holding the P1 Data:
typedef struct P1Data
{
    uint8_t dsmrVersion;         // DSMR versie zonder punt
    double elecUsedT1;           // Elektriciteit verbruikt tarief 1 in kWh
    double elecUsedT2;           // Elektriciteit verbruikt tarief 2 in kWh
    double elecDeliveredT1;      // Elektriciteit geleverd tarief 1 in kWh
    double elecDeliveredT2;      // Elektriciteit geleverd tarief 2 in kWh
    uint8_t currentTarrif;       // Huidig tafief
    double elecCurrentUsage;     // Huidig elektriciteitsverbruik In Watt
    double elecCurrentDeliver;   // Huidig elektriciteit levering in Watt
    double gasUsage;             // Gasverbruik in dm3
    char timeGasMeasurement[14]; // Tijdstip waarop gas voor het laats is gemeten YY:MM:DD:HH:MM:SS
} P1Data;
//Error types for P1 data reading:
#define P1_READ_OK 0
#define P1_ERROR_DSMR_NOT_FOUND 1
#define P1_ERROR_ELECUSEDT1_NOT_FOUND 2
#define P1_ERROR_ELECUSEDT2_NOT_FOUND 3
#define P1_ERROR_ELECRETURNT1_NOT_FOUND 4
#define P1_ERROR_ELECRETURNT2_NOT_FOUND 5
#define P1_ERROR_GAS_READING_NOT_FOUND 6

/**
 *  ========== FUNCTIONS ================
 */
//Init

void initP1UART();
void initGPIO();

//P1 port read and parsing

unsigned int CRC16(unsigned int crc, unsigned char *buf, int len);
int p1StringToStruct(const char *p1String, P1Data *p1Struct);
void printP1Error(int errorType);

//JSON

char *packageESPNowMessageJSON(ESP_message *);
char *packageP1MessageJSON(P1Data *);
void printP1Data(P1Data *data);

//HTTPS

int postESPNOWbackoffice(char *JSONpayload);
int postP1Databackoffice(char *JSONpayload);


#endif //ifndef _P1CONFIG_H