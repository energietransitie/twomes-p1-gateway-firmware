#include <driver/uart.h>
#include <driver/gpio.h>

static const char *TAGP1 = "P1-Config";

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
    uint8_t intervalTime;                       //Interval between measurements, for timestamping
    int16_t pipeTemps1[MAX_SAMPLES_ESPNOW];     //measurements of the first temperature sensor
    int16_t pipeTemps2[MAX_SAMPLES_ESPNOW];     //measurements of the second temperature sensor
};
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
};

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

int packageESPNowMessageJSON(struct ESP_message *message) {
    const char JSONformat[] = "{\"upload_time\": \"%ld\",\"property_measurements\":[{\"property_name\":  \"%s\",\"timestamp\":\"%ld\", \"timestamp_type\", \"end\", \"interval\": %u, \"measurements\": [ \"%s\"]}]}";
    long now = time(NULL); //Get current time

    switch (message->measurementType) {
        case BOILERTEMP:
        {
            ESP_LOGI("JSON", "Received BOILERTEMP, containing %d measurements\n", message->numberofMeasurements);
            //Post boilertemp 1:
            { //Place in scope to free stack after post
                char propertyName[] = "BoilerTemp1";
                char measurements[(6 * MAX_SAMPLES_ESPNOW) + 1]; //Print every temperature as "25.72,34.68,..." resulting in 6 chars for every measurements (4 numbers, a '.' and a ',');
                uint8_t i;
                measurements[0] = 0;
                for (i = 0; i < message->numberofMeasurements; i++) {
                    char measurementString[7];
                    sprintf(measurementString, "%2.2f,", ((float)((message->pipeTemps1[i]) * 0.0078125f)));
                    strcat(measurements, measurementString);
                }//for(i<numberofMeasurements)
                char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //DO NOT FORGET TO FREE AFTER SENDING! should this be on stack instead, with HTTPS post inside this function?
                sprintf(stringifiedMessage, JSONformat, now, propertyName, now, message->intervalTime, measurements);
                ESP_LOGI("JSON", "%s\n", stringifiedMessage);
                free(stringifiedMessage);
            }//post boiler temp 1
            //Post boilertemp 2:
            {
                char propertyName[] = "BoilerTemp2";
                char measurements[(6 * MAX_SAMPLES_ESPNOW) + 1]; //Print every temperature as "25.72,34.68,..." resulting in 6 chars for every measurements (4 numbers, a '.' and a ',');
                measurements[0] = 0;
                uint8_t i;
                for (i = 0; i < message->numberofMeasurements; i++) {
                    char measurementString[7];
                    sprintf(measurementString, "%2.2f,", ((float)((message->pipeTemps2[i]) * 0.0078125f)));
                    strcat(measurements, measurementString);
                }//for(i<numberofMeasurements)
                char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //DO NOT FORGET TO FREE AFTER SENDING! should this be on stack instead, with HTTPS post inside this function?
                sprintf(stringifiedMessage, JSONformat, now, propertyName, now, message->intervalTime, measurements);
                ESP_LOGI("JSON", "%s\n", stringifiedMessage);
                free(stringifiedMessage);
            }//Post boilertemp 2
            return 1;
            break;
        }//case BOILERTEMP
        case ROOMTEMP:
        {
            return 1;
            break;
        }//case ROOMTEMP
        case CO2:
        {
            return 1;
            break;
        }//case CO2
        default:
        {
            return 0;
            break;
        }//default
    }//switch messagetype
}

// char *packageP1portJSON(P1Data data, int length) {


// }