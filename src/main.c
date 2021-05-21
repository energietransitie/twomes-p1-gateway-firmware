//Generic Twomes Firmware
#include <generic_esp_32.h>
#include <P1Config.h>

//To create the JSON and read the P1 port
#include <freertos/FreeRTOSConfig.h>
//ESP-IDF drivers:
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_now.h>
#include <esp_wifi.h>

// #define DEBUG 1

#define P1_READ_INTERVAL 10000 //Interval to read P1 data in milliseconds

static const char *TAG = "Twomes P1 Gateway ESP32";
static const char *HB = "HeartBeat";

// #define EXAMPLE

//Interrupt Queue Handler:
static xQueueHandle gpio_evt_queue = NULL;

//Gpio ISR handler:
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}//gpio_isr_handler

//function to read P1 port and store message in a buffer
void read_P1(void *args) {
    while (1) {
        gpio_set_level(PIN_DRQ, 0); //DRQ pin has inverter to pull up to 5V, which makes it active low
        vTaskDelay(200 / portTICK_PERIOD_MS); //Give some time to let data transmit
        gpio_set_level(PIN_DRQ, 1); //Write DRQ pin low again (otherwise P1 port keeps transmitting every second);

        //Allcoate a buffer with the size of the P1 UART buffer:
        uint8_t *data = (uint8_t *)malloc(P1_BUFFER_SIZE);
        //Read data on UART2:
        int len = uart_read_bytes(P1PORT_UART_NUM, data, P1_BUFFER_SIZE, 20 / portTICK_PERIOD_MS);
        //struct P1Data p1data =  parseP1Data(data);
        //int result = packageP1DataJSON(p1data);

        vTaskDelay((P1_READ_INTERVAL - 200) / portTICK_PERIOD_MS);
    }//while(1)
} //void read_P1

#if DEBUG
//Simple heartbeat print to serial monitor every 5 seconds
void pingAlive(void *args) {
    while (1) {
        ESP_LOGI(HB, "I am alive!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}//void pingAlive
#endif

/**Blink LEDs to test GPIO:
 * Pass two arguments in uint8_t array:
 * argument[0] = amount of blinks
 * argument[1] = pin to blink on (LED_STATUS or LED_ERROR)
 */
void blink(void *args) {
    uint8_t *arguments = (uint8_t *)args;
    uint8_t amount = arguments[0];
    uint8_t pin = arguments[1];
    uint8_t i;
    for (i = 0; i < amount; i++) {
        gpio_set_level(pin, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        gpio_set_level(pin, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }//for(i<amount)
    //Delete the blink task after completion:
    vTaskDelete(NULL);
}//void blink;

/**
 * Check for input of buttons and the duration
 * if the press duration was more than 5 seconds, erase the flash memory to restart provisioning
 * otherwise, blink the status LED (and possibly run another task (sensor provisioning?))
*/
void buttonPressDuration(void *args) {
    uint32_t io_num;
    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            uint8_t seconds = 0;
            while (!gpio_get_level(BUTTON_P2)) {
                vTaskDelay(500 / portTICK_PERIOD_MS);
                seconds++;
                if (seconds == 9) {
                    ESP_LOGI("ISR", "Button held for over 5 seconds\n");
                    char blinkArgs[2] = { 5, LED_ERROR };
                    xTaskCreatePinnedToCore(blink, "blink longpress", 768, (void *)blinkArgs, 10, NULL, 1);
                    //Long press on P2 is for full Reset, clearing provisioning memory:
                    nvs_flash_erase();
                    vTaskDelay(1000 / portTICK_PERIOD_MS); //Wait for blink to finish
                    esp_restart();  //software restart, to get new provisioning. Sensors do NOT need to be paired again when gateway is reset (MAC address does not change) 
                    break; //Exit loop
                }//if (seconds == 9)
                //If the button gets released before 5 seconds have passed:
                else if (gpio_get_level(BUTTON_P2)) {
                    char blinkArgs[2] = { 5, LED_STATUS };
                    xTaskCreatePinnedToCore(blink, "blink shortpress", 768, (void *)blinkArgs, 10, NULL, 1);
                }
            }//while(!gpio_level)
        } //if(xQueueReceive)
    } //while(1)
} //task buttonDuration

//Callback function when receiving ESP-Now data:
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length) {
    uint8_t espnowBlinks[2] = { 2, LED_STATUS }; //Blink status LED twice on receive:
    xTaskCreatePinnedToCore(blink, "blinkESPNOW", 786, espnowBlinks, 1, NULL, 1);
    ESP_LOGI(TAG, "RECEIVED ESP_NOW MESSAGE");

    struct ESP_message ESPNowData;
    memcpy(&ESPNowData, payload, length);
    int result = packageESPNowMessageJSON(&ESPNowData);
    // ESP_LOGI("ESPNOW", "%s", JSONMessage);
}

void app_main(void) {
#if EXAMPLE
    initialize_nvs();
    initialize();
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_prov_mgr_config_t config = initialize_provisioning();
    //Starts provisioning if not provisioned, otherwise skips provisioning.
    start_provisioning(config);
    //Initialize time with timezone Europe and city Amsterdam
    initialize_time("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
    //URL Do not forget to use https:// when using the https() function.
    char *url = "https://192.168.178.75:4444/set/house/opentherm";

    //Gets time as epoch time.
    int now = time(NULL);

    //Creates data string replacing %d with the time integer.
    char *dataPlain = "{\"deviceMac\":\"8C:AA:B5:85:A2:3D\",\"measurements\": [{\"property\":\"testy\",\"value\":\"hello_world\"}],\"time\":%d}";
    char data[strlen(dataPlain)];
    sprintf(data, dataPlain, now);

    /* Start main application now */
    while (1) {
        //Logs hello world and tries to post with https every 10 seconds. Replace post_https(url, data, cert) with post_http(url,data) to use plain http over TCP
        ESP_LOGI(TAG, "Hello World!");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        post_https(url, data, NULL);
    }
#endif
    //INIT NVS and GPIO:
    initialize_nvs();
    initialize();
    initP1UART(); //Should this just be done once in main?
    initGPIO();
    gpio_set_level(PIN_DRQ, 1); //P1 data read is active low.
    uart_flush_input(P1PORT_UART_NUM);  //Empty the buffer from junk that might be received at boot
    //Attach interrupt handler to GPIO pins:
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreatePinnedToCore(buttonPressDuration, "buttonPressDuration", 2048, NULL, 10, NULL, 1);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_P2, gpio_isr_handler, (void *)BUTTON_P2);

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    //Start Provisioning:
    wifi_prov_mgr_config_t config = initialize_provisioning();
    //Starts provisioning if not provisioned, otherwise skips provisioning.
    start_provisioning(config);
    //Initialize time with timezone Europe and city Amsterdam
    initialize_time("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
    //URL Do not forget to use https:// when using the https() function.
    const char *url = TWOMES_P1_URL;
    //Gets time as epoch time.
    long now = time(NULL);
    //Log the time:
    ESP_LOGI(TAG, "%ld", now);

    //Set to station mode for ESP-Now
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //Check if disconnecting and reconnecting works:
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // ESP_ERROR_CHECK(esp_wifi_connect());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // ESP_LOGI("P1", "DISCONNECTING FROM WIFI");
    // ESP_ERROR_CHECK(esp_wifi_disconnect());
    // uint8_t channel;
    // esp_wifi_get_channel(&channel, WIFI_SECOND_CHAN_NONE);
    // wifi_sta_config_t wificonfig;
    // esp_wifi_get_config(ESP_IF_WIFI_STA, &wificonfig);
    // ESP_LOGI("WIFI", "Connected to WiFi %s on channel %d\n", (const char *)wificonfig.ssid, channel);
    //Initialise ESP-Now
    ESP_ERROR_CHECK(esp_now_init());
    uint8_t macAddress[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, macAddress);
    ESP_LOGI("MAC", "My MAC address is: %02X:%02X:%02X:%02X:%02X:%02X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    esp_now_register_recv_cb(onDataReceive);

    //Create tasks:
    xTaskCreatePinnedToCore(read_P1, "uart_read_p1", 4096, NULL, 10, NULL, 1);
#if DEBUG
    xTaskCreatePinnedToCore(pingAlive, "ping_alive", 2048, NULL, 10, NULL, 1);
#endif
}