#define DEBUG 1

//Generic Twomes Firmware
#include "generic_esp_32.h"

#ifdef CONFIG_WITH_ROOMCO2_SATELLITE
    #ifdef CONFIG_WITH_BOILER_SATELLITE
    const char *device_type_name = "DSMR-P1-gateway-TinTsTrCO2";
    #else
    const char *device_type_name = "DSMR-P1-gateway-TinCO2";
    #endif
#elif CONFIG_WITH_ROOM_SATELLITE
    #ifdef CONFIG_WITH_BOILER_SATELLITE
    const char *device_type_name = "DSMR-P1-gateway-TinTsTr";
    #else
    const char *device_type_name = "DSMR-P1-gateway-Tin";
    #endif
#else
    #ifdef CONFIG_WITH_BOILER_SATELLITE
    const char *device_type_name = "DSMR-P1-gateway-TsTr";
    #else
    const char *device_type_name = "DSMR-P1-gateway";
    #endif
#endif

static const char *TAG = "Twomes P1 Gateway ESP32";

#define BOOT_STARTUP_INTERVAL_MS (10 * 1000) // milliseconds ( 10 s * 1000 ms/s)
#define BOOT_STARTUP_INTERVAL_TXT "Wating 10 seconds before next measurement data series is started"

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


#define P1_MEASUREMENT_INTERVAL_MS 5 * 60 * 1000 //milliseconds (5 min * 60  s/min * 1000 ms/s)

#define DEBUGHEAP //Prints free heap size to serial port on a fixed interval


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
// void blink(void *args);
void buttonPressHandler(void *args);
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length);
void read_P1(void *args);
// TODO move this to P1Config if possible:
void postESPNOWbackoffice(void *args);
void postP1backoffice(void *args);
void espnow_available_task(void *args);

/* =============== MAIN ============== */
void app_main(void) {
    //INIT:
    initP1UART();                   //Setup P1 UART
    initGPIO_P1();                     //Setup GPIO

    //Attach interrupt handler to GPIO pins:
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //Create interrupt handler task:
    xTaskCreatePinnedToCore(buttonPressHandler, "handle_button_pressed", 4096, NULL, 10, NULL, 1);
    gpio_install_isr_service(0);
    //Attach pushbuttons to gpio ISR handler:
    // gpio_isr_handler_add(BUTTON_SW2, gpio_isr_handler, (void *)BUTTON_SW2);
    gpio_isr_handler_add(BUTTON_GPIO12_SW3, gpio_isr_handler, (void *)BUTTON_GPIO12_SW3);


    gpio_set_level(PIN_DRQ, 1);        //P1 data read is active low.
    uart_flush_input(P1PORT_UART_NUM); //Empty the buffer from data that might be received before PIN_DRQ got pulled high

    //Setup generic Firmware: V2.0.0 now contains all initialising functions inside provisioning setup
    twomes_device_provisioning(device_type_name);

    //Set to station mode for ESP-Now
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    //Get the channel from NVS, if not yet stored, select a channel and save it:
    uint8_t espNowChannel = manageEspNowChannel();
    uint8_t wifiChannel;
    wifi_second_chan_t secondWiFiChannel;
    esp_err_t err_wifi = esp_wifi_get_channel(&wifiChannel, &secondWiFiChannel);

    //Throw an error if WiFi channel could not be read:
    if (err_wifi != ESP_OK) ESP_LOGE("MAIN", "Could not read WiFi channel: %s", esp_err_to_name(err_wifi));

    //Log the channel used for Wi-Fi and for ESP-Now:
    ESP_LOGI("MAIN", "Connected to WiFi on channel %u, and using channel %u for ESP-Now!", wifiChannel, espNowChannel);



    //Create "forever running" tasks:
    ESP_LOGD(TAG, "Starting heartbeat task");
    xTaskCreatePinnedToCore(&heartbeat_task, "twomes_heartbeat", 16384, NULL, 1, NULL, 1);

    xTaskCreatePinnedToCore(read_P1, "uart_read_p1", 16384, NULL, 10, NULL, 1);

    xTaskCreatePinnedToCore(espnow_available_task, "espnow_poll", 4096, NULL, 11, NULL, 1);

    ESP_LOGD(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Starting timesync task");
    xTaskCreatePinnedToCore(&timesync_task, "timesync_task", 4096, NULL, 1, NULL, 1);

    #ifdef CONFIG_TWOMES_PRESENCE_DETECTION
    ESP_LOGD(TAG, BOOT_STARTUP_INTERVAL_TXT);
    vTaskDelay(BOOT_STARTUP_INTERVAL_MS / portTICK_PERIOD_MS);
    
    ESP_LOGD(TAG, "Starting presence detection");
    start_presence_detection();
    #endif

#if defined(DEBUGHEAP)
    xTaskCreatePinnedToCore(pingHeap, "ping_Heap", 2048, NULL, 1, NULL, 1);
#endif
}

/* ============ TASKS AND FUNCTIONS =========== */

//function to read P1 port and store message in a buffer
void read_P1(void *args) {
    while (1) {
        //Read the current time to compensate the delay:
        int64_t lP1ReadStartTime = esp_timer_get_time();

        //Empty the buffer before requesting data to clear it of junk
        uart_flush(P1PORT_UART_NUM);
        ESP_LOGI("P1", "Attempting to read P1 Port");
        //DRQ pin has inverter to pull up to 5V, which makes it active low:      
        gpio_set_level(PIN_DRQ, 0);
        //Wait for 18 seconds to ensure a message is read even on a DSMR4.x device:
        vTaskDelay(18000 / portTICK_PERIOD_MS);
        //Write DRQ pin low again (otherwise P1 port keeps transmitting every second);
        gpio_set_level(PIN_DRQ, 1);

        //Allcoate a buffer with the size of the P1 UART buffer to store P1 data:
        uint8_t *data = (uint8_t *)malloc(P1_BUFFER_SIZE);

        //Read data from the P1 UART:
        int len = uart_read_bytes(P1PORT_UART_NUM, data, P1_BUFFER_SIZE, 20 / portTICK_PERIOD_MS);

        //If data is received:
        if (len > 0) {
            //Trim the received message to contain only the necessary data and store the CRC as an unsigned int:
            char *p1MessageStart = strchr((const char *)data, '/'); //Find the position of the start-of-message character ('/')
            char *p1MessageEnd = NULL;
            //Only look for end if a start is found:
            if (p1MessageStart != NULL) p1MessageEnd = strchr((const char *)p1MessageStart, '!');   //Find the position of the end-of-message character ('!')

            //Check if a message is received:
            if (p1MessageEnd != NULL) {

                //Convert the CRC from string to int:
                unsigned int receivedCRC;
                //Start the scanf one char after the end-of-message symbol (location of CRC16), and read a 4-symbol hex number
                sscanf(p1MessageEnd + 1, "%4X", &receivedCRC);
                //Allocate memory to copy the trimmed message into
                uint8_t *p1Message = malloc(P1_MESSAGE_SIZE);
                //Trim the message to only include 1 full P1 port message:
                len = (int) (p1MessageEnd - p1MessageStart) + 1;
                memcpy(p1Message, p1MessageStart, len);
                p1Message[len] = 0; //Add zero terminator to end of message
                ESP_LOGD("P1", "Trimmed message length: %d bytes)", len);

                //Calculate the CRC of the trimmed message:
                unsigned int calculatedCRC = CRC16(0x0000, p1Message, len);
                #ifdef DSMR2OR3
                  receivedCRC = calculatedCRC;
                #endif
                //Check if CRC match:
                if (calculatedCRC == receivedCRC) {
                    //log received CRC and calculated CRC for debugging
                    ESP_LOGD("P1", "Received matching CRC: (%4X == %4X)", receivedCRC, calculatedCRC);
                    ESP_LOGI("P1", "Parsing message into struct:");
                    //Create a struct for the p1 measurements
                    P1Data p1Measurements;
                    //extract the necessary data from the P1 payload into the struct and check for errors while decoding
                    int result = p1StringToStruct((const char *)p1Message, &p1Measurements);

                    if (result == P1_READ_OK) {
                        //Print the data from the struct to monitor for debugging:
                        printP1Data(&p1Measurements);

                        //put the data in the JSON string:
                        char *p1JSON = packageP1MessageJSON(&p1Measurements);

                        //Post the data to the backoffice:
                        xTaskCreatePinnedToCore(postP1backoffice, "P1-POST", 4096, (void *)p1JSON, 10, NULL, 1);
                    }
                    else {
                        //If a measurement could not be read, print to serial terminal which one was (the first that was) missing
                        printP1Error(result);
                        //Blink error LED twice to indicate error to user
                        char blinkArgs[2] = { 2, RED_LED_D1_ERROR };
                        xTaskCreatePinnedToCore(blink, "CRCerrorBlink", 768, (void *)blinkArgs, 10, NULL, 1);
                    }
                    //Start decoding the P1 message:
                } //if(calculatedCRC == receivedCRC)
                //if CRC does not match:
                else {
                    //Log received and calculated CRC for debugging and flash the Error LED
                    ESP_LOGE("ERROR - P1", "CRC DOES NOT MATCH");
                    ESP_LOGD("ERROR - P1", "Received CRC %4X but calculated CRC %4X", receivedCRC, calculatedCRC);

                    //Blink error LED twice
                    char blinkArgs[2] = { 2, RED_LED_D1_ERROR };
                    xTaskCreatePinnedToCore(blink, "CRCerrorBlink", 768, (void *)blinkArgs, 10, NULL, 1);
                } //else (CRC check)

                //Free the P1 message from memory
                free(p1Message);
            }//if(p1MessageEnd != NULL)
            else {
                ESP_LOGE("P1", "P1 message was invalid");
            }
        } else {
            ESP_LOGI("P1", "No P1 message found");
        }
        //if len>0;
        //Release the data from the memory buffer:
        free(data);

        int64_t lTimeAfterP1Read = esp_timer_get_time();
        int64_t lTimeDiffMilliSeconds = (lTimeAfterP1Read - lP1ReadStartTime) / 1000;

        vTaskDelay((P1_MEASUREMENT_INTERVAL_MS - lTimeDiffMilliSeconds) / portTICK_PERIOD_MS); //This should be calibrated to check for the time spent calculating the data
    } //while(1) - Never ending Task
} //void read_P1

#if defined(DEBUGHEAP)
//For debugging: print the amount of free memory every minute
void pingHeap(void *args) {
    while (1) {
        ESP_LOGD("Heap", "I am alive! Free heap size: %u bytes", esp_get_free_heap_size());
        vTaskDelay(1 * 60 * 1000 / portTICK_PERIOD_MS);
    }
} //void pingAlive
#endif

 /**
  * Check for input of buttons and the duration
  * if the press duration was more than 10 seconds, erase the flash memory to restart provisioning
  * otherwise, blink the status LED (and possibly run another task (sensor provisioning?))
 */
void buttonPressHandler(void *args) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            vTaskDelay(100 / portTICK_PERIOD_MS); //Debounce delay: nothing happens if user presses button for less than 100 ms = 0.1s

            //INTERRUPT HANDLER BUTTON SW3
            if (io_num == BUTTON_GPIO12_SW3) {
                uint8_t halfSeconds = 0;
                //Determine length of button press before taking action
                while (!gpio_get_level(BUTTON_GPIO12_SW3)) {
                    //Button SW3 is (still) pressed
                    vTaskDelay(500 / portTICK_PERIOD_MS); //Wait for 0.5s
                    halfSeconds++;
                    ESP_LOGD(TAG, "Button SW3 has been pressed for %u half-seconds now", halfSeconds);
                    if (halfSeconds >= LONG_BUTTON_PRESS_DURATION) {
                        //Long press on SW3 is for clearing Wi-Fi provisioning memory:
                        ESP_LOGI("ISR", "Long-button press detected on SW3; resetting Wi-Fi provisioning and restarting device");
                        char blinkArgs[2] = { 5, RED_LED_D1_ERROR };
                        xTaskCreatePinnedToCore(blink, "blink longpress", 768, (void *)blinkArgs, 10, NULL, 1);
                        esp_wifi_restore();
                        vTaskDelay(5 * (200+200) * 1000 / portTICK_PERIOD_MS); //Wait 1s for blink to finish
                        esp_restart();                         //software restart, to enable linking to new Wi-Fi router. Sensors do NOT need to be paired again: MAC address of P1-gateway does not change)
                        break;                                 //Exit loop (this should not be reached)
                    }                                          //if (halfSeconds == 9)
                    else if (gpio_get_level(BUTTON_GPIO12_SW3)) {
                        //Button SW3 is released
                        //Short press on SW3 is for coupling with a satellite by sending the ESP-NOW channel number on channel 0:
                        ESP_LOGI("ISR", "Short button press detected on SW3");
                        char blinkArgs[2] = { 5, GREEN_LED_D2_STATUS };
                        xTaskCreatePinnedToCore(blink, "blink_green_LED_5_times", 768, (void *)blinkArgs, 10, NULL, 1);
                        xTaskCreatePinnedToCore(sendEspNowChannel, "pair_sensor", 2048, NULL, 15, NULL, 1); //Send data in relatively high priority task
                    }
                } //while(!gpio_level)
            }
        }     //if(xQueueReceive)
    }  //while(1)
} // buttonPressHandler

//Callback function when receiving ESP-Now data:
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length) {

    uint8_t espnowBlinks[2] = { 2, GREEN_LED_D2_STATUS }; //Blink status LED twice on receive:
    xTaskCreatePinnedToCore(blink, "blinkESPNOW", 786, espnowBlinks, 1, NULL, 1);
    ESP_LOGI(TAG, "RECEIVED ESP_NOW MESSAGE");

    //Move received data into ESP-Message struct and package to JSON
    struct ESP_message ESPNowData;
    memcpy(&ESPNowData, payload, length);
    char *measurementsJSON = packageESPNowMessageJSON(&ESPNowData);

    //Create task to POST
    xTaskCreatePinnedToCore(postESPNOWbackoffice, "ESPNOW-POST", 4096, (void *)measurementsJSON, 10, NULL, 1);

    ESP_LOGI("ESPNOW", "ESP-Now Callback ended"); //check if callback ends after creating task
}

/**
 * @brief poll 802_11 semaphore to see if resource is available
 * release semaphore after taking to
 */
void espnow_available_task(void *args) {
    bool espNowIsEnabled = false;
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //if esp-Now is off and the semaphore is available, turn on ESP-Now:
        if (!espNowIsEnabled) {
            if (xSemaphoreTake(wireless_802_11_mutex, 10 / portTICK_PERIOD_MS)) {
                ESP_LOGI("ESP-NOW", "Taking semaphore and enabling ESP-Now");
                //SetupEsPNow() also releases Semaphore:
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                ESP_ERROR_CHECK(esp_now_init());
                esp_now_register_recv_cb(onDataReceive);
                p1ConfigSetupEspNow();
                //Initialise ESP-Now
                //Register the ESP-Now receive callback
                //Switch to the ESP-Now channel:
                espNowIsEnabled = true;
            }
            else ESP_LOGI("ESP-Now", "Semaphore was not available");
        }
        //If semaphore is taken and ESP-Now is enabled, deinit esp-now to ensure no data collisions (already should not happen, but just in case?)
        if (espNowIsEnabled) {
            if (xSemaphoreTake(wireless_802_11_mutex, 10 / portTICK_PERIOD_MS)) {
                xSemaphoreGive(wireless_802_11_mutex);
            }
            else {
                ESP_LOGI("ESP-NOW", "Sempahore is taken, releasing resources, disabling ESP-Now");
                esp_now_unregister_recv_cb();
                esp_now_deinit();
                espNowIsEnabled = false;
            }

        }
    }
}