#include "P1Config.h"
#define LOG_LOCAL_LEVEL 3
#define DSMR22 1 
/**
 * @brief Initialise UART "P1PORT_UART_NUM" for P1 receive
 */
void initP1UART() {
    //UART Configuration for P1-Port reading:
    #if defined(DSMR22)
    //9600 baud 7E1, even parity
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_7_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    #else
    //115200 baud, 8n1, no parity, no HW flow control
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    #endif
    ESP_ERROR_CHECK(uart_param_config(P1PORT_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(P1PORT_UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_line_inverse(P1PORT_UART_NUM, UART_SIGNAL_RXD_INV | UART_SIGNAL_IRDA_RX_INV)); //Invert RX data
    ESP_ERROR_CHECK(uart_driver_install(P1PORT_UART_NUM, P1_BUFFER_SIZE * 2, 0, 0, NULL, 0));
}

/**
 * @brief Initalise pushbuttons, LEDs and Data-Request pin
 */
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
char *packageESPNowMessageJSON(ESP_message *message) {
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
            {                                                    //Place in scope to free stack after post
                char measurements[(8 * MAX_SAMPLES_ESPNOW) + 1]; //Allocate 8 chars per measurement, (5 for value,  1 comma seperator and 2 apostrophes)
                char measurements2[(8 * MAX_SAMPLES_ESPNOW) + 1];
                uint8_t i;
                measurements[0] = 0; //initialise 0 on first element of array to indicate start of string
                measurements2[0] = 0;
                struct Boiler_message {
                    int16_t pipeTemps1[MAX_SAMPLES_ESPNOW];
                    int16_t pipeTemps2[MAX_SAMPLES_ESPNOW];
                } boilerMessage;
                memcpy(&boilerMessage, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
                for (i = 0; i < message->numberofMeasurements; i++) {
                    if (i == message->numberofMeasurements - 1) { //Print the last measurement without comma
                        char measurementString[9];
                        sprintf(measurementString, "\"%2.2f\"", ((float)((boilerMessage.pipeTemps1[i]) * 0.0078125f)));
                        strcat(measurements, measurementString);
                        sprintf(measurementString, "\"%2.2f\"", ((float)((boilerMessage.pipeTemps2[i]) * 0.0078125f)));
                        strcat(measurements2, measurementString);
                    }
                    else {
                        char measurementString[9];
                        sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps1[i]) * 0.0078125f)));
                        strcat(measurements, measurementString);
                        sprintf(measurementString, "\"%2.2f\",", ((float)((boilerMessage.pipeTemps2[i]) * 0.0078125f)));
                        strcat(measurements2, measurementString);
                    }
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
                char measurements[(8 * MAX_TEMP_SAMPLES) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
                uint8_t i;
                measurements[0] = 0; //initialise 0 on first element of array to indicate start of string
                uint16_t roomTemps[120];
                memcpy(roomTemps, message->data, sizeof(message->data)); //copy data from uint8_t[240] to datatype of measurement. Could this be typecast instead of copy?
                for (i = 0; i < message->numberofMeasurements; i++) {
                    if (i == message->numberofMeasurements - 1) { //Print last measurement without comma
                        char measurementString[9];
                        sprintf(measurementString, "\"%2.2f\"", ((float)(((175.72f) * roomTemps[i]) / 65536) - 46.85f));
                        strcat(measurements, measurementString);
                    }
                    else {
                        char measurementString[9];
                        sprintf(measurementString, "\"%2.2f\",", ((float)(((175.72f) * roomTemps[i]) / 65536) - 46.85f));
                        strcat(measurements, measurementString);
                    }
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
            const char JSONformat[] = "\"property_measurements\":["
                "{\"property_name\":\"CO2concentration\","
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]},"

                "{\"property_name\":\"relativeHumidity\","
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]},"

                "{\"property_name\":\"roomTempCO2\","
                "\"timestamp\":\"%ld\","
                "\"timestamp_type\":\"end\","
                "\"interval\":%u,"
                "\"measurements\":[%s]}"
                "]}";
            long now = time(NULL); //Get current time
            ESP_LOGI("JSON", "Received CO2, containing %d measurements\n", message->numberofMeasurements);
            char measurementsppm[(6 * MAX_CO2_SAMPLES) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
            measurementsppm[0] = 0;                             //initialise 0 on first element of array to indicate start of string
            char measurementsTemp[(6 * MAX_CO2_SAMPLES) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
            measurementsTemp[0] = 0;                             //initialise 0 on first element of array to indicate start of string
            char measurementsRH[(6 * MAX_CO2_SAMPLES) + 1]; //Allocate 8 chars per measurement, (5 for value, 1 comma seperator and  2 apostrophes)
            measurementsRH[0] = 0;
            struct CO2_message {
                uint16_t scd41ppm[MAX_CO2_SAMPLES];
                uint16_t scd41temp[MAX_CO2_SAMPLES];
                uint16_t scd41rh[MAX_CO2_SAMPLES];
            } co2EspNowMessage;

            memcpy(&co2EspNowMessage, message->data, sizeof(message->data));
            uint8_t i;
            for (i = 0; i < message->numberofMeasurements; i++) {
                if (i == message->numberofMeasurements - 1) { //Print last measurement without comma
                    char measurementString[9];
                    sprintf(measurementString, "\"%hd\"", co2EspNowMessage.scd41ppm[i]);
                    strcat(measurementsppm, measurementString);
                }
                else {
                    char measurementString[9];
                    sprintf(measurementString, "\"%hd\",", co2EspNowMessage.scd41ppm[i]);
                    strcat(measurementsppm, measurementString);
                }
            } //for(i<numberofMeasurements)
            for (i = 0; i < message->numberofMeasurements; i++) {
                if (i == message->numberofMeasurements - 1) { //Print last measurement without comma
                    char measurementString[9];
                    sprintf(measurementString, "\"%2.2f\"", (-45 + 175 * co2EspNowMessage.scd41temp[i] / 65536.0f));
                    strcat(measurementsTemp, measurementString);
                }
                else {
                    char measurementString[9];
                    sprintf(measurementString, "\"%2.2f\",", (-45 + 175 * co2EspNowMessage.scd41temp[i] / 65536.0f));
                    strcat(measurementsTemp, measurementString);
                }
            } //for(i<numberofMeasurements)
            for (i = 0; i < message->numberofMeasurements; i++) {
                if (i == message->numberofMeasurements - 1) { //Print last measurement without comma
                    char measurementString[9];
                    sprintf(measurementString, "\"%3.1f\"", (100 * co2EspNowMessage.scd41rh[i] / 65536.0f));
                    strcat(measurementsRH, measurementString);
                }
                else {
                    char measurementString[9];
                    sprintf(measurementString, "\"%3.1f\",", (100 * co2EspNowMessage.scd41rh[i] / 65536.0f));
                    strcat(measurementsRH, measurementString);
                }
            } //for(i<numberofMeasurements)
            char *stringifiedMessage = malloc(JSON_BUFFER_SIZE); //Do not forget to free after sending! Should this be on stack? or return pointer to this from function and handle HTTPS POST in seperate function? (should then remove upload time insertion here...)
            sprintf(stringifiedMessage, JSONformat, now, message->intervalTime, measurementsppm, now, message->intervalTime, measurementsTemp, now, message->intervalTime, measurementsRH);
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


/**
 * @brief Package P1 Measurements message into Twomes JSON format
 *
 * @param data the P1 data struct
 *
 * @return pointer to JSON string on heap (NEEDS TO BE FREED)
 */
char *packageP1MessageJSON(P1Data *data) {
    //Get current time:
    unsigned int now = time(NULL);
    char *JSONformat =
        "\"property_measurements\": ["
        "{\"property_name\": \"eMeterReadingSupplyLow\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","   //Timestamp of P1 read
        "\"value\": \"%.3f\"}"       //Double
        "]},"
        "{\"property_name\": \"eMeterReadingSupplyHigh\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%.3f\"}"
        "]},"
        "{\"property_name\": \"eMeterReadingReturnLow\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%.3f\"}"
        "]},"
        "{\"property_name\": \"eMeterReadingReturnHigh\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%.3f\"}"
        "]},"
        "{\"property_name\": \"eMeterReadingTimestamp\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%13s\"}"  //13 character string
        "]},"
        "{\"property_name\": \"gMeterReadingSupply\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%.3f\"}"
        "]},"
        "{\"property_name\": \"gMeterReadingTimestamp\","
        "\"measurements\": ["
        "{\"timestamp\": \"%u\","
        "\"value\": \"%13s\"}"  //13 character string
        "]}"
        "]}";
    char *P1JSONbuffer = malloc(JSON_BUFFER_SIZE);
    //print measurement data into JSON string:
    sprintf(P1JSONbuffer, JSONformat, \
        now, data->elecUsedT1, \
        now, data->elecUsedT2, \
        now, data->elecDeliveredT1, \
        now, data->elecDeliveredT2, \
        now, data->timeElecMeasurement, \
        now, data->gasUsage, \
        now, data->timeGasMeasurement);
    return P1JSONbuffer;
}


/**
 * @brief calculate the CRC16 of the P1 message (A001 polynomial)
 *
 * @param crc starting value of the crc, to allow for cumulative CRC (init with 0x0000 when scanning full message at once)
 * @param buf pointer to the string to calculate the crc on
 * @param len the length of the string
 *
 * @return the calculated CRC16
 */
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len) {
    for (int pos = 0; pos < len; pos++) {
        crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) { // Loop over each bit
            if ((crc & 0x0001) != 0) {// If the LSB is set
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
int p1StringToStruct(const char *p1String, P1Data *p1Struct) {
    //Use strstr() from string library to find OBIS references to datatypes of P1 message
    //See https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf at page 20 for definitions:

    //strstr() returns null when string is not found, can be used to check for errors

    //REVIEW: should packaging into struct be omitted and directly package into JSON format?ol

    //DSMR version: OBIS reference 1-3:0.2.8
    char *dsmrPos = strstr(p1String, "1-3:0.2.8");
    if (dsmrPos != NULL) {
        //Read the version number:
        sscanf(dsmrPos, "1-3:0.2.8(%hhu", &(p1Struct->dsmrVersion)); //Read DSMR version as unsigned char
    }
    //else
      //  return P1_ERROR_DSMR_NOT_FOUND; //DSMR version not found is not an error

    //elecUsedT1 OBIS reference: 1-0:1.8.1
    char *elecUsedT1Pos = strstr(p1String, "1-0:1.8.1");
    if (elecUsedT1Pos != NULL) {
        //read the ElecUsedT1, specification states fixed 3 decimal float:
        sscanf(elecUsedT1Pos, "1-0:1.8.1(%lf", &(p1Struct->elecUsedT1));
    }
    else
        return P1_ERROR_ELECUSEDT1_NOT_FOUND;

    //elecUsedT2 OBIS reference: 1-0:1.8.2
    char *elecUsedT2Pos = strstr(p1String, "1-0:1.8.2");
    if (elecUsedT2Pos != NULL) {
        sscanf(elecUsedT2Pos, "1-0:1.8.2(%lf", &p1Struct->elecUsedT2);
    }
    else
        return P1_ERROR_ELECUSEDT2_NOT_FOUND;

    //elecReturnT1 OBIS reference: 1-0:2.8.1
    char *elecReturnT1Pos = strstr(p1String, "1-0:2.8.1");
    if (elecReturnT1Pos != NULL) {
        sscanf(elecReturnT1Pos, "1-0:2.8.1(%lf", &p1Struct->elecDeliveredT1);
    }
    else
        return P1_ERROR_ELECRETURNT2_NOT_FOUND;


    //elecReturnT2 OBIS reference 1-0:2.8.2
    char *elecReturnT2Pos = strstr(p1String, "1-0:2.8.2");
    if (elecReturnT2Pos != NULL) {
        sscanf(elecReturnT2Pos, "1-0:2.8.2(%lf", &p1Struct->elecDeliveredT2);
    }
    else
        return P1_ERROR_ELECRETURNT2_NOT_FOUND;

    //elec Timestamp OBIS reference 
    char *elecTimePos = strstr(p1String, "0-0:1.0.0");
    if (elecTimePos != NULL) {
        sscanf(elecTimePos, "0-0:1.0.0(%13s", p1Struct->timeElecMeasurement);
        p1Struct->timeElecMeasurement[13] = 0; //add a zero terminator at the end to read as string
    } 
#if defined(DSMR22)
    //DSMR 2.2 had different layout of gas timestap and gas reading
    //Gas reading OBIS: 0-n:24.3.0 //n can vary depending on which channel it is installed
    char *gasTimePos = strstr(p1String, "0-1:24.3.0");
    if (gasTimePos != NULL) {
        sscanf(gasTimePos, "0-1:24.3.0(%12s)", p1Struct->timeGasMeasurement);
        p1Struct->timeGasMeasurement[12] = 0; //Add a null terminator to print it as a string
    }
    else
        return P1_ERROR_GAS_READING_NOT_FOUND;
    char *gasPos = strstr(p1String, "m3)");
    if (gasTimePos != NULL) {
        sscanf(gasPos, "m3)\n(%lf)", &p1Struct->gasUsage);
    }
    else
        return P1_ERROR_GAS_READING_NOT_FOUND;
#else
//Gas reading OBIS: 0-n:24.2.1 //n can vary depending on which channel it is installed
    char *gasPos = strstr(p1String, "0-1:24.2.1");
    if (gasPos != NULL) {
        sscanf(gasPos, "0-1:24.2.1(%13s)(%lf)", p1Struct->timeGasMeasurement, &p1Struct->gasUsage);
        p1Struct->timeGasMeasurement[13] = 0; //Add a null terminator to print it as a string
    }
    else
        return P1_ERROR_GAS_READING_NOT_FOUND;
#endif
    //If none of the statements reached an "else" all measurements were read correctly!
    return P1_READ_OK;
}

/**
 * @brief print P1 struct data to UART1/LOGI
 *
 * @param data pointer to P1Data type struct
 *
 */
void printP1Data(P1Data *data) {
    ESP_LOGI("P1 Print", "DSMR VERSION %i ", data->dsmrVersion);
    ESP_LOGI("P1 Print", "ELEC USED T1: %4.3f ", data->elecUsedT1);
    ESP_LOGI("P1 Print", "ELEC USED T2: %4.3f ", data->elecUsedT2);
    ESP_LOGI("P1 Print", "ELEC RETURNED T1: %4.3f ", data->elecDeliveredT1);
    ESP_LOGI("P1 Print", "ELEC RETURNED T2: %4.3f ", data->elecDeliveredT2);
    ESP_LOGI("P1 Print", "ELEC TIMESTAMP: %s", data->timeElecMeasurement);
    ESP_LOGI("P1 Print", "GAS USED:  %7.3f ", data->gasUsage);
    ESP_LOGI("P1 Print", "GAS TIMESTAMP: %s ", data->timeGasMeasurement);
}

/**
 * @brief print P1 Error type to serial monitor with explanation
 *
 * @param errorType the errortype returned from parsing P1 data
 *
 */
void printP1Error(int errorType) {
    switch (errorType) {
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
        case P1_ERROR_ELEC_TIMESTAMP_NOT_FOUND:
            ESP_LOGI("P1_ERROR", "Electricity timestamp not found");
            break;
        default:
            break;
    }
}

/**
 * @brief compare function for qsort
 *
 * @param a pointer to first value
 * @param b pointer to second value
 *
 * @return 0 for equal, negative for b>a positive for a>b
 *
 */
int compare(const void *a, const void *b) {
    if (((*(uint8_t *)a) - (*(uint8_t *)b)) > 0) return 1; //if a-b > 0, a is larger than b
    else if (((*(uint8_t *)a) - (*(uint8_t *)b)) < 0) return -1; //if a-b < 0, b is larger than a
    else return 0; //if a-b == 0, a is equal to b
}

/**
 * @brief scan all available Access Points and return their channels
 *
 * @param channelScanList pointer to a channelList struct for storing data
 */
void scanChannels(channelList *channelScanList) {
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI("SCAN", "Total APs scanned = %u", ap_count);
    channelScanList->amount = ap_count;
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        channelScanList->channels[i] = ap_info[i].primary;
        ESP_LOGI("SCAN", "Found Network with SSID %s on primary channel %d and secondary channel %d, with RSSI %d", ap_info[i].ssid, channelScanList->channels[i], (int)ap_info[i].second, ap_info[i].rssi);
    }
    esp_wifi_scan_stop();
    // qsort(channelScanList->channels, channelScanList->amount, sizeof(uint8_t), compare);
}

/**
 * @brief count how many acces points are using each channel
 *
 * @param channels list of channels found using scanChannels()
 *
 * @return pointer to array containing amount of APs on each channel
 */
uint8_t *countChannels(channelList *channels) {
    uint8_t *channelOccurrances = malloc(AMOUNT_WIFI_CHANNELS);
    uint8_t occurancesIndex = 0;
    uint8_t count = 0;
    for (occurancesIndex = 0; occurancesIndex < AMOUNT_WIFI_CHANNELS; occurancesIndex++) {
        for (uint8_t i = 0; i < channels->amount; i++) {
            //Channels start at 1, so add 1 to occurancesIndex
            //If the channel is the same as the index, add one to count
            if (channels->channels[i] == (occurancesIndex + 1)) {
                count++;
            }
        }
        channelOccurrances[occurancesIndex] = count;
        count = 0;
    }
    return channelOccurrances;
}

/**
 * @brief algorithm to find the lowest number in an array
 *
 * @param channels list returned by countChannels()
 *
 * @return the least used channel
 */
uint8_t findMinimum(uint8_t *channels) {
    uint8_t position = 0;
    uint8_t smallest = channels[0];
    for (uint8_t i = 1; i < AMOUNT_WIFI_CHANNELS; i++) {
        if (smallest > channels[i]) {
            smallest = channels[i];
            position = i;
        }
    }
    return position + 1; //Add one, since the 0 in array is channel 1
}

/**
 * @brief retrieve the channel to use for ESP-Now, and if none is found, generate a new one
 *
 * @return the channel to use for ESP-Now retrieved from NVS
 */
uint8_t manageEspNowChannel() {
    esp_err_t err;
    uint8_t channel;
    nvs_handle_t channelHandle;
    err = nvs_open("twomes_storage", NVS_READWRITE, &channelHandle);
    if (err != ESP_OK) {
        ESP_LOGE("CHANNEL", "Failed to open NVS twomes_storage: %s", esp_err_to_name(err));
        return 0;
    }

    err = nvs_get_u8(channelHandle, "espnowchannel", &channel);
    switch (err) {
        case ESP_OK:
            ESP_LOGI("CHANNEL", "Read channel %u from NVS!", channel);
            return channel;
        case ESP_ERR_NVS_NOT_FOUND:
        {
            //If not found, generate a channel
            channelList channels;
            scanChannels(&channels);
            uint8_t *channelOccurances = countChannels(&channels);
            for (uint8_t i = 0; i < AMOUNT_WIFI_CHANNELS; i++) {
                ESP_LOGI("SCAN", "Channel %u occurs %u times", i + 1, channelOccurances[i]);
            }
            channel = findMinimum(channelOccurances);
            ESP_LOGI("SCAN", "The least occuring channel is: %u, now storing to NVS", channel);
            err = nvs_set_u8(channelHandle, "espnowchannel", channel);
            if (err == ESP_OK) {
                nvs_commit(channelHandle);
                ESP_LOGI("CHANNEL", "Succesfully wrote channel to NVS");
                return channel;
            }
            else {
                ESP_LOGE("CHANNEL", "Failed to write channel to twomes_storage: %s", esp_err_to_name(err));
                return 0;
            }
        }
        default:
            ESP_LOGE("CHANNEL", "Error (%s) reading!\n", esp_err_to_name(err));
            return 0;
    }
}

/**
 * @brief sends the ESP-Now channel and MAC address to a sensor using ESP-Now broadcast on WiFi channel 1
 *
 */
void sendEspNowChannel(void *args) {
    uint8_t channel = manageEspNowChannel(); //Get channel from NVS, should this be given as a param from RAM?

    disconnect_wifi("ESP-Now send pairing info"); //Disable the Generic Twomes Auto-Connect feature, to allow channel switching
    esp_now_init(); //Initialise ESP-Now

    //Set the channel to the channel devices use for pairing:
    esp_wifi_set_channel(ESPNOW_PAIRING_CHANNEL, WIFI_SECOND_CHAN_NONE);

    uint8_t primary;
    wifi_second_chan_t secondary;
    esp_wifi_get_channel(&primary, &secondary); //Read the channel to validate we changed it
    ESP_LOGI("ESPNOW", "Now on WiFi channel %u", primary);
    uint8_t broadcastAddress[6] = { 0xff,0xff,0xff,0xff,0xff,0xff };
    esp_now_peer_info_t broadCastPeer;
    memcpy(broadCastPeer.peer_addr, broadcastAddress, 6); //Copy the broadcast address to the peer
    esp_now_add_peer(&broadCastPeer);
    //Using a broadcast, using a callBack function for checking is not possible, use LEDs on sensor to indicate receiving a message!
    // esp_now_register_send_cb(sendChannelCallback);
    esp_err_t err = esp_now_send(broadcastAddress, &channel, 1); //Only need to send channel, MAC can be read from message already
    if (err != ESP_OK) ESP_LOGI("ESPNOW-PAIR", "ERROR sending ESP-Now channel: %s", esp_err_to_name(err));
    else ESP_LOGI("ESPNOW-PAIR", "Sent channel with succes!");

    //End the RTOS task:
    vTaskDelete(NULL);
}

/**
 * @brief turn off ESP-Now capabilities and switch to Wi-Fi
 *
 * @param reason string for task name to help debugging
 *
 * @return succes status
 */
int p1ConfigSetupWiFi(char *reason) {
    //Re-enable Wi-Fi through Generic Twomes Firmware, should automatically swap channel on connecting
    if (connect_wifi(reason)) return 0;
    return -1;
}

/**
 * @brief disconnect from Wi-Fi and turn on ESP-Now
 *
 * @return succes status
 */
int p1ConfigSetupEspNow() {
    if (disconnect_wifi("ESP-Now listen")) {
        vTaskDelay(500 / portTICK_PERIOD_MS); //Give time for Wi-Fi to disconnect
        uint8_t channel = manageEspNowChannel(); //Get the channel from NVS
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        return 0;
    }
    else return -1;
}

/**
 * @brief Post ESP-Now data to the backoffice (fixed interval)
 *
 * @param data JSON stringified payload, typecast to void*
 */
void postESPNOWbackoffice(void *args) {
    //p1ConfigSetupWiFi("ESP-Now message POST"); //connect to Wi-Fi
    //Short delay to get Wi-Fi set up
    //vTaskDelay(1000 / portTICK_PERIOD_MS);

    //Add the time of posting to the JSON message
    time_t now = time(NULL);
    char *postJSON = malloc(JSON_BUFFER_SIZE);
    postJSON[0] = 0; //initalise first element with 0 for printing
    sprintf(postJSON, "{\"upload_time\": \"%ld\",", now);
    postJSON = strcat(postJSON, (char *)args);
    ESP_LOGI("ESPNOW", "Sending JSON to backoffice: %s", postJSON);

    //Post the JSON data to the backoffice
    char response[128] = "";
    int responsecode = upload_data_to_server((const char *)FIXED_INTERVAL_URL, POST_WITH_BEARER, postJSON, response, sizeof response);
    ESP_LOGI("HTTPS", "Response code: %d: %s", responsecode, response);

    //Delay to give Wi-Fi time to finish up after post
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //Free all strings from heap 
    //TODO: Keep "args" (JSON data without timestamp) for buffering
    free(args);
    free(postJSON); //Gets freed in post_https function (This should not be the case?)
    vTaskDelete(NULL); //Self destruct
}

/**
 * @brief Post P1 Data to the backoffice (Variable interval)
 *
 * @param data JSON stringified payload, typecast to void*
 */
void postP1backoffice(void *args) {
    //p1ConfigSetupWiFi("Post P1 Data"); //connect to Wi-Fi
    //Short delay to get Wi-Fi set up
    //vTaskDelay(1000 / portTICK_PERIOD_MS);

    //Add the time of posting to the JSON message
    time_t now = time(NULL);
    char *postJSON = malloc(JSON_BUFFER_SIZE);
    postJSON[0] = 0; //initalise first element with 0 for printing
    //Add the time to the JSON string:
    sprintf(postJSON, "{\"upload_time\": \"%ld\",", now);
    //concatenate the measurements onto the timestamp
    postJSON = strcat(postJSON, (char *)args);

    //Print to terminal for debugging:
    ESP_LOGI("ESPNOW", "Sending JSON to backoffice: %s", postJSON);

    /**===Post the JSON data to the backoffice===**/
    //create buffer to store the response
    char response[128] = "\0";
    //POST
    int responsecode = upload_data_to_server(VARIABLE_INTERVAL_URL, POST_WITH_BEARER, postJSON, response, sizeof(response));
    //Print response and code:
    ESP_LOGI("HTTPS", "Response code: %d: %s", responsecode, response);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //Free all strings from heap 

    //TODO: Keep "args" (JSON data without timestamp) for buffering
    free(args);
    free(postJSON);
    //Switch back to ESP-Now after post

    vTaskDelete(NULL); //Self destruct
}