#define DEBUG 1
#define CUSTOM_IO 1
//Generic Twomes Firmware
#include "generic_esp_32.h"
#include "certificate.h"
//To create the JSON and read the P1 port
#include "P1Config.h"
#include <string.h>

//ESP-IDF drivers:
#include <freertos/FreeRTOSConfig.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <wifi_provisioning/manager.h>

char *bearer;
const char *device_activation_url = TWOMES_TEST_SERVER"/device/activate";
const char *variable_interval_upload_url = TWOMES_TEST_SERVER"/device/measurements/variable-interval";
const char *fixed_interval_upload_url = TWOMES_TEST_SERVER"/device/measurements/fixed-interval";

#define P1_READ_INTERVAL 60000 //Interval to read P1 data in milliseconds

#define DEVICE_NAME "Generic-Test"
#define HEARTBEAT_UPLOAD_INTERVAL 3600000 //ms, so one hour
#define HEARTBEAT_MEASUREMENT_INTERVAL 600000 //ms, so 10 minutes; not yet in effect


#define DEBUGHEAP //Prints free heap size to serial port on a fixed interval

static const char *TAG = "Twomes P1 Gateway ESP32";

//Interrupt Queue Handler:
static xQueueHandle gpio_evt_queue = NULL;

//Gpio ISR handler:
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
} //gpio_isr_handler



//Functions:
#if defined(DEBUGHEAP)
void pingHeap(void *);
#endif
void blink(void *args);
void buttonPressDuration(void *args);
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length);
void read_P1(void *args);
void postESPNOWbackoffice(void *args);

/* =============== MAIN ============== */
void app_main(void) {
    //INIT NVS and GPIO:
    initialize_nvs();
    initialize();
    initP1UART(); //Should this just be done once in main?
    initGPIO();
    gpio_set_level(PIN_DRQ, 1);        //P1 data read is active low.
    uart_flush_input(P1PORT_UART_NUM); //Empty the buffer from junk that might be received at boot
    //Attach interrupt handler to GPIO pins:
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreatePinnedToCore(buttonPressDuration, "buttonPressDuration", 2048, NULL, 10, NULL, 1);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_P2, gpio_isr_handler, (void *)BUTTON_P2);

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());


    prepare_device();

    wifi_prov_mgr_config_t config = initialize_provisioning();
    //Starts provisioning if not provisioned, otherwise skips provisioning.
    //If set to false it will not autoconnect after provisioning.
    //If set to true it will autonnect.
    start_provisioning(config, true);
    //Initialize time with timezone Europe and city Amsterdam
    initialize_time("UTC");
    //Gets time as epoch time.
    long now = time(NULL);
    //Log the time:
    ESP_LOGI(TAG, "%ld", now);

    bearer = get_bearer();
    if (strlen(bearer) > 1) {
        ESP_LOGI(TAG, "Bearer read: %s", bearer);
    }
    else if (strcmp(bearer, "") == 0) {
        ESP_LOGI(TAG, "Bearer not found, activating device!");
        activate_device(device_activation_url, DEVICE_NAME, rootCAR3);
        bearer = get_bearer();
    }
    else if (!bearer) {
        ESP_LOGE(TAG, "Something went wrong whilst reading the bearer!");
    }

    //Set to station mode for ESP-Now
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //Check if disconnecting and reconnecting works:
    ESP_ERROR_CHECK(esp_now_init());
    uint8_t macAddress[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, macAddress);
    ESP_LOGI("MAC", "My MAC address is: %02X:%02X:%02X:%02X:%02X:%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    //Now that internet is available, collect the bearer:


    //Register the ESP-Now receive callback
    esp_now_register_recv_cb(onDataReceive);
    enable_wifi();
    //Create tasks:
    xTaskCreatePinnedToCore(read_P1, "uart_read_p1", 4096, NULL, 10, NULL, 1);
#if defined(DEBUGHEAP)
    xTaskCreatePinnedToCore(pingHeap, "ping_Heap", 2048, NULL, 1, NULL, 1);
#endif
}

/* ============ TASKS AND FUNCTIONS =========== */

//function to read P1 port and store message in a buffer
void read_P1(void *args) {
    while (1) {
        uart_flush(P1PORT_UART_NUM);          //Empty the buffer before requesting data to clear it of junk
        gpio_set_level(PIN_DRQ, 0);           //DRQ pin has inverter to pull up to 5V, which makes it active low
        vTaskDelay(200 / portTICK_PERIOD_MS); //Give some time to let data transmit. 115200 baud with max 1kB data = max 70ms data transmission
        gpio_set_level(PIN_DRQ, 1);           //Write DRQ pin low again (otherwise P1 port keeps transmitting every second);

        //Allcoate a buffer with the size of the P1 UART buffer:
        uint8_t *data = (uint8_t *)malloc(P1_BUFFER_SIZE);
        //Read data on UART2:
        int len = uart_read_bytes(P1PORT_UART_NUM, data, P1_BUFFER_SIZE, 20 / portTICK_PERIOD_MS);
        if (len > 0) { //only perform check if data is received
            //Trim the received message to contain only the necessary data and store the CRC as an unsigned int:
            char *p1MessageStart = strchr((const char *)data, '/'); //Find the position of the start-of-message character ('/')
            char *p1MessageEnd = strchr((const char *)data, '!');   //Find the position of the end-of-message character ('!')
            //Convert the CRC from string to int:
            unsigned int receivedCRC;
            sscanf(p1MessageEnd + 1, "%4X", &receivedCRC); //Start the scanf one char after the end-of-message symbol, and read a 4-symbol hex number
            uint8_t *p1Message = malloc(P1_BUFFER_SIZE);   //Allocate memory to copy the trimmed message into
            memcpy(p1Message, p1MessageStart, (p1MessageEnd - p1MessageStart) + 1);
            p1Message[p1MessageEnd - p1MessageStart + 1] = 0; //Add zero terminator to end of message
            free(data);                                       //Free the original read data

            //Calculate the CRC of the trimmed message:
            unsigned int calculatedCRC = CRC16(0x0000, p1Message, (int)(p1MessageEnd - p1MessageStart + 1));
            //Check if CRC match:
            if (calculatedCRC == receivedCRC) {
                //log received CRC and calculated CRC for debugging
                ESP_LOGI("P1", "Received matching CRC: (%4X == %4X)", receivedCRC, calculatedCRC);
                ESP_LOGI("P1", "Parsing message into struct:");
                //Create a struct for the p1 measurements
                P1Data p1Measurements;
                //extract the necessary data from the P1 payload into the struct and check for errors while decoding
                int result = p1StringToStruct((const char *)p1Message, &p1Measurements);
                ESP_LOGI("P1", "%s", p1Message);
                ESP_LOGI("P1", "\n\nMeasurement decoded with result %i\n\n", result);
                if (result == P1_READ_OK)
                    printP1Data(&p1Measurements);
                else {
                    //If a measurement could not be read, print to serial terminal which one was (the first that was) missing
                    printP1Error(result);
                    //Blink error LED twice to indicate error to user
                    char blinkArgs[2] = { 2, LED_ERROR };
                    xTaskCreatePinnedToCore(blink, "CRCerrorBlink", 768, (void *)blinkArgs, 10, NULL, 1);
                }
                //Start decoding the P1 message:
            } //if(calculatedCRC == receivedCRC)
            //if CRC does not match:
            else {
                //Log received and calculated CRC for debugging and flash the Error LED
                ESP_LOGI("ERROR - P1", "CRC DOES NOT MATCH");
                ESP_LOGI("ERROR - P1", "Received CRC %4X but calculated CRC %4X", receivedCRC, calculatedCRC);
                //Blink error LED twice
                char blinkArgs[2] = { 2, LED_ERROR };
                xTaskCreatePinnedToCore(blink, "CRCerrorBlink", 768, (void *)blinkArgs, 10, NULL, 1);
            } //else (CRC check)
            free(p1Message);
        }//if len>0;
        else free(data);
        //Free the P1 message memory
        vTaskDelay((P1_READ_INTERVAL - 200) / portTICK_PERIOD_MS); //This should be calibrated to check for the time spent calculating the data
    } //while(1)
} //void read_P1

#if defined(DEBUGHEAP)
//Simple heartbeat print to serial monitor every 10 seconds
//Prints size of free heap as well
void pingHeap(void *args) {

    while (1) {
        ESP_LOGI("Heap", "I am alive! Free heap size: %u bytes", esp_get_free_heap_size());
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
} //void pingAlive
#endif

/**Blink LEDs to test GPIO
 * @param args Pass two arguments in uint8_t array
 * @param argument[0] amount of blinks
 * @param argument[1] pin to blink on (LED_STATUS or LED_ERROR)
 */
void blink(void *args) {
    uint8_t *arguments = (uint8_t *)args;
    uint8_t pin = arguments[1];
    uint8_t amount = arguments[0];
    uint8_t i;
    for (i = 0; i < amount; i++) {
        gpio_set_level(pin, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(pin, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    } //for(i<amount)
    //Delete the blink task after completion:
    vTaskDelete(NULL);
} //void blink;

/**
 * Check for input of buttons and the duration
 * if the press duration was more than 5 seconds, erase the flash memory to restart provisioning
 * otherwise, blink the status LED (and possibly run another task (sensor provisioning?))
*/
void buttonPressDuration(void *args) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            uint8_t halfSeconds = 0;
            while (!gpio_get_level(BUTTON_P2)) {
                vTaskDelay(500 / portTICK_PERIOD_MS);
                halfSeconds++;
                if (halfSeconds == 19) {
                    ESP_LOGI("ISR", "Button held for over 10 seconds\n");
                    char blinkArgs[2] = { 5, LED_ERROR };
                    xTaskCreatePinnedToCore(blink, "blink longpress", 768, (void *)blinkArgs, 10, NULL, 1);
                    //Long press on P2 is for clearing provisioning memory:
                    esp_wifi_restore();
                    vTaskDelay(1000 / portTICK_PERIOD_MS); //Wait for blink to finish
                    esp_restart();                         //software restart, to get new provisioning. Sensors do NOT need to be paired again when gateway is reset (MAC address does not change)
                    break;                                 //Exit loop (this should not be reached)
                }                                          //if (halfSeconds == 9)
                //If the button gets released before 5 seconds have passed:
                else if (gpio_get_level(BUTTON_P2)) {
                    char blinkArgs[2] = { 5, LED_STATUS };
                    xTaskCreatePinnedToCore(blink, "blink shortpress", 768, (void *)blinkArgs, 10, NULL, 1);
                }
            } //while(!gpio_level)
        }     //if(xQueueReceive)
    }         //while(1)
} //task buttonDuration

//Callback function when receiving ESP-Now data:
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length) {
    uint8_t espnowBlinks[2] = { 2, LED_STATUS }; //Blink status LED twice on receive:
    xTaskCreatePinnedToCore(blink, "blinkESPNOW", 786, espnowBlinks, 1, NULL, 1);
    ESP_LOGI(TAG, "RECEIVED ESP_NOW MESSAGE");
    struct ESP_message ESPNowData;
    memcpy(&ESPNowData, payload, length);
    char *measurementsJSON = packageESPNowMessageJSON(&ESPNowData);
    time_t now = time(NULL);
    char *postJSON = malloc(JSON_BUFFER_SIZE);
    postJSON[0] = 0; //initalise first element with 0 for printing
    sprintf(postJSON, "{\"upload_time\": \"%ld\",", now);
    postJSON = strcat(postJSON, measurementsJSON);
    ESP_LOGI("ESPNOW", "Sending JSON to backoffice: %s", postJSON);
    xTaskCreatePinnedToCore(postESPNOWbackoffice, "ESPNOW-POST", 4096, (void *)postJSON, 10, NULL, 1);
    free(measurementsJSON); //TODO this should only be freed after successfull post, otherwise kept for buffering
    ESP_LOGI("ESPNOW", "ESP-Now Callback ended"); //check if callback ends after creating task
}


/**
 * @brief Post ESP-Now data to the backoffice (fixed interval)
 *
 * @param data JSON stringified payload
 *
 * @return http code
 */
void postESPNOWbackoffice(void *args) {
    char *message = (char *)args;
    char *response = post_https(fixed_interval_upload_url, message, rootCAR3, bearer);
    ESP_LOGI("HTTPS", "Response: %s", response);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    free(message);
    free(response);
    vTaskDelete(NULL); //Self destruct
}