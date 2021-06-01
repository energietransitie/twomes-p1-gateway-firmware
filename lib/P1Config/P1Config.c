#include "P1Config.h"

//Function to initialise the P1 Port UART Receiver, 115200 Baud 8N1, no flow control:
void initP1UART()
{
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
    ESP_ERROR_CHECK(uart_set_line_inverse(P1PORT_UART_NUM, UART_SIGNAL_RXD_INV | UART_SIGNAL_IRDA_RX_INV)); //Invert RX data
    ESP_ERROR_CHECK(uart_driver_install(P1PORT_UART_NUM, P1_BUFFER_SIZE * 2, 0, 0, NULL, 0));
}

//Function to initialise the buttons and LEDs on the gateway, with interrupts on the buttons
void initGPIO()
{
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
char *packageESPNowMessageJSON(ESP_message *message)
{
    switch (message->measurementType)
    {
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
        {                                                    //Place in scope to free stack after post
            char measurements[(8 * MAX_SAMPLES_ESPNOW) + 1]; //Allocate 8 chars per measurement, (5 for value,  1 comma seperator and 2 apostrophes)
            char measurements2[(8 * MAX_SAMPLES_ESPNOW) + 1];
            uint8_t i;
            measurements[0] = 0; //initialise 0 on first element of array to indicate start of string
            measurements2[0] = 0;
            struct Boiler_message
            {
                int16_t pipeTemps1[MAX_SAMPLES_ESPNOW];
                int16_t pipeTemps2[MAX_SAMPLES_ESPNOW];
            } boilerMessage;
            memcpy(&boilerMessage, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
            for (i = 0; i < message->numberofMeasurements; i++)
            {
                char measurementString[9];
                sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps1[i]) * 0.0078125f)));
                strcat(measurements, measurementString);
                sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps2[i]) * 0.0078125f)));
                strcat(measurements2, measurementString);
            }                                                    //for(i<numberofMeasurements)
            char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
            sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements, now, message->intervalTime, measurements2);
            //HTTPS_post(stringifiedmessage);
            ESP_LOGI("JSON", "%s\n", stringifiedMessage);
            return stringifiedMessage;
        } //post boiler temp
        break;
    } //case BOILERTEMP
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
        {                                                    //Place in scope to free stack after post
            char measurements[(8 * MAX_SAMPLES_ESPNOW) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
            uint8_t i;
            measurements[0] = 0; //initialise 0 on first element of array to indicate start of string
            uint16_t roomTemps[120];
            memcpy(roomTemps, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
            for (i = 0; i < message->numberofMeasurements; i++)
            {
                char measurementString[9];
                //Currently using conversion of DS18B20 RAW temperature! (No SI7051 support yet!)
                sprintf(measurementString, "\"%2.2f\",", ((float)((roomTemps[i]) * 0.0078125f)));
                strcat(measurements, measurementString);
            }                                                    //for(i<numberofMeasurements)
            char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
            sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements);
            ESP_LOGI("JSON", "%s\n", stringifiedMessage);
            return stringifiedMessage;
            break;
        }
    } //case ROOMTEMP
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
        measurements[0] = 0;                             //initialise 0 on first element of array to indicate start of string
        uint16_t co2ppm[120];
        memcpy(co2ppm, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement.  Could this be typecast instead of copy?
        uint8_t i;
        for (i = 0; i < message->numberofMeasurements; i++)
        {
            char measurementString[9];
            sprintf(measurementString, "\"%d\",", co2ppm[i]);
            strcat(measurements, measurementString);
        }                                                    //for(i<numberofMeasurements)
        char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
        sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurements);
        //HTTPS_post(stringifiedmessage);               //not yet working
        ESP_LOGI("JSON", "%s\n", stringifiedMessage);
        return stringifiedMessage;
        break;
    } //case CO2
    default:
    {
        ESP_LOGI("JSON", "Received an unknown type");
        return 0;
        break;
    } //default
    } //switch messagetype
}

// char *packageP1portJSON(P1Data data, int length) {

// }

/**
 * @brief Post ESP-Now data to the backoffice (fixed interval)
 * 
 * @param data JSON stringified payload
 * 
 * @return http code
 */
int postESPNOWbackoffice(char *JSONpayload)
{
    return 0;
}

/**
 * @brief Post P1 port data to the backoffice (fixed interval)
 * 
 * @param data JSON stringified payload
 * 
 * @return http code
 */
int postP1Databackoffice(char *JSONpayload)
{
    return 0;
}

//Returns CRC
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--)
        { // Loop over each bit
            if ((crc & 0x0001) != 0)
            {              // If the LSB is set
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else           // Else LSB is not set
                crc >>= 1; // Just shift right
        }
    }
    return crc;
}

/**
 * @brief Convert data read from P1 port into the struct
 *
 * @param p1String String received from the P1 port
 * @param p1Struct pointer to struct to save the data in
 *
 * @return Success (0) or error for missing datatype for error-check
 *
 */
int p1StringToStruct(const char *p1String, P1Data *p1Struct)
{
    //Use strstr() from string library to find OBIS references to datatypes of P1 message
    //See https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf at page 20 for definitions:

    //strstr() returns null when string is not found, can be used to check for errors

    //REVIEW: should packaging into struct be omitted and directly package into JSON format?ol

    //DSMR version: OBIS reference 1-3:0.2.8
    char *dsmrPos = strstr(p1String, "1-3:0.2.8");
    if (dsmrPos != NULL)
    {
        //Read the version number:
        sscanf(dsmrPos, "1-3:0.2.8(%hhui", &(p1Struct->dsmrVersion)); //Read DSMR version as unsigned char
    }
    else
        return P1_ERROR_DSMR_NOT_FOUND; //DSMR version not found

    //elecUsedT1 OBIS reference: 1-0:1.8.1
    char *elecUsedT1Pos = strstr(p1String, "1-0:1.8.1");
    if (elecUsedT1Pos != NULL)
    {
        //read the ElecUsedT1, specification states fixed 3 decimal float:
        sscanf(elecUsedT1Pos, "1-0:1.8.1(%lf", &(p1Struct->elecUsedT1));
    }
    else
        return P1_ERROR_ELECUSEDT1_NOT_FOUND;

    //elecUsedT2 OBIS reference: 1-0:1.8.2
    char *elecUsedT2Pos = strstr(p1String, "1-0:1.8.2");
    if (elecUsedT2Pos != NULL)
    {
        sscanf(elecUsedT2Pos, "1-0:1.8.2(%lf", &p1Struct->elecUsedT2);
    }
    else
        return P1_ERROR_ELECUSEDT2_NOT_FOUND;

    //elecReturnT1 OBIS reference: 1-0:2.8.1
    char *elecReturnT1Pos = strstr(p1String, "1-0:2.8.1");
    if (elecReturnT1Pos != NULL)
    {
        sscanf(elecReturnT1Pos, "1-0:2.8.1(%lf", &p1Struct->elecDeliveredT1);
    }
    else
        return P1_ERROR_ELECRETURNT2_NOT_FOUND;

    //elecReturnT2 OBIS reference 1-0:2.8.2
    char *elecReturnT2Pos = strstr(p1String, "1-0:2.8.2");
    if (elecReturnT2Pos != NULL)
    {
        sscanf(elecReturnT2Pos, "1-0:2.8.2(%lf", &p1Struct->elecDeliveredT2);
    }
    else
        return P1_ERROR_ELECRETURNT2_NOT_FOUND;

    //Gas reading OBIS: 0-n:24.2.1 //n can vary depending on which channel it is installed
    char *gasPos = strstr(p1String, "0-1:24.2.1");
    if (gasPos != NULL)
    {
        sscanf(gasPos, "0-1:24.2.1(%13s)(%lf)", p1Struct->timeGasMeasurement, &p1Struct->gasUsage);
        p1Struct->timeGasMeasurement[13] = 0; //Add a null terminator to print it as a string
    }
    else
        return P1_ERROR_GAS_READING_NOT_FOUND;

    //If none of the statements reached an "else" all measurements were read correctly!
    return P1_READ_OK;
}

/**
 * @brief print P1 struct data to UART1/LOGI
 * 
 * @param *data pointer to P1Data type struct
 * 
 */
void printP1Data(P1Data *data)
{
    ESP_LOGI("P1 Print", "DSMR VERSION %i ", data->dsmrVersion);
    ESP_LOGI("P1 Print", "ELEC USED T1: %4.3f ", data->elecUsedT1);
    ESP_LOGI("P1 Print", "ELEC USED T2: %4.3f ", data->elecUsedT2);
    ESP_LOGI("P1 Print", "ELEC RETURNED T1: %4.3f ", data->elecDeliveredT1);
    ESP_LOGI("P1 Print", "ELEC RETURNED T2: %4.3f ", data->elecDeliveredT2);
    ESP_LOGI("P1 Print", "GAS USED:  %7.3f ", data->gasUsage);
    ESP_LOGI("P1 Print", "GAS TIMESTAMP: %s ", data->timeGasMeasurement);
}

/**
 * @brief print P1 Error type to serial monitor with explanation
 * 
 * @param errorType the errortype returned from parsing P1 data
 * 
 */
void printP1Error(int errorType)
{
    switch (errorType)
    {
    case P1_ERROR_DSMR_NOT_FOUND:
        ESP_LOGI("P1 ERROR", "DSMR version could not be found");
        break;
    case P1_ERROR_ELECUSEDT1_NOT_FOUND:
        ESP_LOGI("P1_ERROR", "Electricity used Tariff 1 not found");
        break;
    case P1_ERROR_ELECUSEDT2_NOT_FOUND:
        ESP_LOGI("P1_ERROR", "Electricity used Tariff 2 not found");
        break;
    case P1_ERROR_ELECRETURNT1_NOT_FOUND:
        ESP_LOGI("P1_ERROR", "Electricity returned Tariff 1 not found");
        break;
    case P1_ERROR_ELECRETURNT2_NOT_FOUND:
        ESP_LOGI("P1_ERROR", "Electricity returned Tariff 2 not found");
        break;
    case P1_ERROR_GAS_READING_NOT_FOUND:
        ESP_LOGI("P1_ERROR", "Gas reading not found");
        break;
    default:
        break;
    }
}