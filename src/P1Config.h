#include <driver/uart.h>
#include <driver/gpio.h>

static const char *TAGP1 = "P1-Config";

char *bearer;
const char *rootCAR3 = "-----BEGIN CERTIFICATE-----\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
"-----END CERTIFICATE-----\n";

//UART defines
#define P1_BUFFER_SIZE 1024
#define P1PORT_UART_NUM UART_NUM_2

//DeviceTypes
#define DEVICETYPE_P1_ONLY          "DSMR-P1-gateway"
#define DEVICETYPE_P1_WITH_SENSORS  "DSMR-P1-gateway-TinTsTr"

//HTTP and JSON
#define TWOMES_P1_URL "localhost:8000"
#define ACTIVATE_ADDRESS "/device/activate"
#define DATA_ADDRESS "/device/measurements/fixed-interval"
#define JSON_BUFFER_SIZE 4096

//Pin definitions:
#define BUTTON_P1   GPIO_NUM_0
#define BUTTON_P2   GPIO_NUM_12
#define LED_ERROR   GPIO_NUM_13
#define LED_STATUS  GPIO_NUM_14
#define PIN_DRQ     GPIO_NUM_17
#define OUTPUT_BITMASK ((1ULL<<LED_ERROR)|(1ULL<<LED_STATUS)|(1ULL<<PIN_DRQ))
#define INPUT_BITMASK ((1ULL << BUTTON_P1)|(1ULL<<BUTTON_P2))

#define MAX_SAMPLES_ESPNOW 60
//Types of measurements that can be received through ESP-Now:
enum ESPNOWdataTypes {
    BOILERTEMP,
    ROOMTEMP,
    CO2,
};
//Types of data that can be read from the P1 Port:
enum P1PORTdataTypes {
    eMETER_SUPPLY_LOW,
    eMETER_SUPPLY_HIGH,
    eMETER_RETURN_LOW,
    eMETER_RETURN_HIGH,
    gMETER__SUPPLY,
    eMETER_TIMESTAMP,
};

//Struct should be exact same as in Measurement device, measurement type is enumerated in ESPNOWdataTypes
typedef struct ESP_message {
    uint8_t measurementType;       //Type of measurement
    uint8_t numberofMeasurements;               //number of measurements
    uint16_t index;                             //Number identifying the message, only increments on receiving an ACK from Gateway
    uint16_t intervalTime;                       //Interval between measurements, for timestamping
    uint8_t data[240];
}ESP_message;

//Struct for holding the P1 Data:
typedef struct P1Data {
    uint8_t dsmrVersion;         // DSMR versie zonder punt
    long elecUsedT1;             // Elektriciteit verbruikt tarief 1 in kWh
    long elecUsedT2;             // Elektriciteit verbruikt tarief 2 in kWh
    long elecDeliveredT1;        // Elektriciteit geleverd tarief 1 in kWh
    long elecDeliveredT2;        // Elektriciteit geleverd tarief 2 in kWh
    uint8_t currentTarrif;       // Huidig tafief
    long elecCurrentUsage;       // Huidig elektriciteitsverbruik In Watt
    long elecCurrentDeliver;     // Huidig elektriciteit levering in Watt
    long gasUsage;               // Gasverbruik in dm3
    char timeGasMeasurement[12]; // Tijdstip waarop gas voor het laats is gemeten YY:MM:DD:HH:MM:SS
}P1Data;

//Function to initialise the P1 Port UART Receiver, 115200 Baud 8N1, no flow control:
void initP1UART() {
    //UART Configuration for P1-Port reading:
    //115200 baud, 8n1, no parity, no HW flow control
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(P1PORT_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(P1PORT_UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // ESP_ERROR_CHECK(uart_set_line_inverse(P1PORT_UART_NUM, UART_SIGNAL_RXD_INV | UART_SIGNAL_IRDA_RX_INV)); //Invert RX data
    ESP_ERROR_CHECK(uart_driver_install(P1PORT_UART_NUM, P1_BUFFER_SIZE * 2, 0, 0, NULL, 0));
}

//Function to initialise the buttons and LEDs on the gateway, with interrupts on the buttons
void initGPIO() {
    gpio_config_t io_conf;
    //CONFIGURE OUTPUTS:
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = OUTPUT_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    //CONFIGURE INPUTS:
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = INPUT_BITMASK;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/**
 * @brief Package ESP-Now message into Twomes JSON format
 *
 * @param ESP_message data
 *
 * @return pointer to JSON string on heap (NEEDS TO BE FREED)
 */
char *packageESPNowMessageJSON(struct ESP_message *message) {
    switch (message->measurementType) {
        case BOILERTEMP:
        {
            const char JSONformat[] =
                "\"property_measurements\":"
                "[{\"property_name\":\"boilerTemp1\"," //BoilerTemp1
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]},"

                "{\"property_name\":\"boilerTemp2\"," //BoilerTemp2
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,\""
                "measurements\":[%s]}]}";
            long now = time(NULL); //Get current time
            ESP_LOGI("JSON", "Received BOILERTEMP with index %d, containing %d measurements\n", message->index, message->numberofMeasurements);
            //Post boilertemp 1:
            { //Place in scope to free stack after post
                char measurements[(8 * MAX_SAMPLES_ESPNOW) + 1]; //Allocate 8 chars per measurement, (5 for value,  1 comma seperator and 2 apostrophes)
                char measurements2[(8 * MAX_SAMPLES_ESPNOW) + 1];
                uint8_t i;
                measurements[0] = 0;    //initialise 0 on first element of array to indicate start of string
                measurements2[0] = 0;
                struct Boiler_message {
                    int16_t pipeTemps1[MAX_SAMPLES_ESPNOW];
                    int16_t pipeTemps2[MAX_SAMPLES_ESPNOW];
                }boilerMessage;
                memcpy(&boilerMessage, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
                for (i = 0; i < message->numberofMeasurements; i++) {
                    char measurementString[9];
                    sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps1[i]) * 0.0078125f)));
                    strcat(measurements, measurementString);
                    sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps2[i]) * 0.0078125f)));
                    strcat(measurements2, measurementString);
                }//for(i<numberofMeasurements)
                char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
                sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements, now, message->intervalTime, measurements2);
                //HTTPS_post(stringifiedmessage);
                ESP_LOGI("JSON", "%s\n", stringifiedMessage);
                return stringifiedMessage;
            }//post boiler temp
            break;
        }//case BOILERTEMP
        case ROOMTEMP:
        {
            const char JSONformat[] = "\"property_measurements\":"
                "[{\"property_name\":\"roomTemp\","
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]}]}";
            long now = time(NULL); //Get current time
            ESP_LOGI("JSON", "Received ROOMTEMP with index %d, containing %d measurements\n", message->index, message->numberofMeasurements);
            //Post boilertemp 1:
            { //Place in scope to free stack after post
                char measurements[(8 * MAX_SAMPLES_ESPNOW) + 1];  //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
                uint8_t i;
                measurements[0] = 0;    //initialise 0 on first element of array to indicate start of string
                uint16_t roomTemps[120];
                memcpy(roomTemps, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
                for (i = 0; i < message->numberofMeasurements; i++) {
                    char measurementString[9];
                    //Currently using conversion of DS18B20 RAW temperature! (No SI7051 support yet!)
                    sprintf(measurementString, "\"%2.2f\",", ((float)((roomTemps[i]) * 0.0078125f)));
                    strcat(measurements, measurementString);
                }//for(i<numberofMeasurements)
                char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
                sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements);
                ESP_LOGI("JSON", "%s\n", stringifiedMessage);
                return stringifiedMessage;
                break;
            }
        }//case ROOMTEMP
        case CO2:
        {
            const char JSONformat[] = "\"property_measurements\":"
                "[{\"property_name\":\"CO2concentration\","
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]}]}";
            long now = time(NULL); //Get current time
            ESP_LOGI("JSON", "Received ROOMTEMP, containing %d measurements\n", message->numberofMeasurements);
            char measurements[(6 * MAX_SAMPLES_ESPNOW) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
            measurements[0] = 0;    //initialise 0 on first element of array to indicate start of string
            uint16_t co2ppm[120];
            memcpy(co2ppm, message->data, sizeof(message->data));   //copy data from uint8_t[240] to datatype of measurement.  Could this be typecast instead of copy?
            uint8_t i;
            for (i = 0; i < message->numberofMeasurements; i++) {
                char measurementString[9];
                sprintf(measurementString, "\"%d\",", co2ppm[i]);
                strcat(measurements, measurementString);
            }//for(i<numberofMeasurements)
            char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
            sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements);
            //HTTPS_post(stringifiedmessage);               //not yet working
            ESP_LOGI("JSON", "%s\n", stringifiedMessage);
            return stringifiedMessage;
            break;
        }//case CO2
        default:
        {
            ESP_LOGI("JSON", "Received an unknown type");
            return 0;
            break;
        }//default
    }//switch messagetype
}

// char *packageP1portJSON(P1Data data, int length) {


// }