#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE_Twomes.h>
//#include <BLE2902.h>

class CallbacksFrom_STATE_WIFI_CHAR : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *STATE_WIFI_CHAR)
    {
        Serial.println("WiFi state readed");
    };
};

class CallbacksFrom_SSID_WIFI_CHAR : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *SSID_WIFI_CHAR)
    {
        uint8_t *testDing2;
        testDing2 = SSID_WIFI_CHAR->getData();

        char test1[50];
        uint8_t counter1 = 0;
        do
        {
            test1[counter1] = testDing2[counter1];
            counter1++;
        } while (test1[counter1 - 1] != 0);
        Serial.println("");
        Serial.print("Readed SSID_WIFI: ");
        Serial.println(test1);
    };
};

class CallbacksFrom_PASSWORD_WIFI_CHAR : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *PASSWORD_WIFI_CHAR)
    {
        uint8_t *testDing2;
        testDing2 = PASSWORD_WIFI_CHAR->getData();

        char test1[50];
        uint8_t counter1 = 0;
        do
        {
            test1[counter1] = testDing2[counter1];
            counter1++;
        } while (test1[counter1 - 1] != 0);
        Serial.println("");
        Serial.print("Received from BLE password_WIFI: ");
        Serial.println(test1);
    };
};

class CallbacksFrom_UNIQUE_ID_GATEWAY_CHAR : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *UNIQUE_ID_GATEWAY_CHAR)
    {
        Serial.println("Unique ID GATEWAY readed");
    };

    void onWrite(BLECharacteristic *UNIQUE_ID_GATEWAY_CHAR)
    {
        Serial.println("Unique ID GATEWAY writed");
    };
};

class CallbacksFrom_UNIQUE_ID_BOILER_CHAR : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *UNIQUE_ID_BOILER_CHAR)
    {
        Serial.println("Unique ID BOILER readed");
    };
};

class CallbacksFrom_UNIQUE_ID_ROOM_CHAR : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *UNIQUE_ID_ROOM_CHAR)
    {
        Serial.println("Unique ID ROOM readed");
    };
    void onWrite(BLECharacteristic *UNIQUE_ID_ROOM_CHAR)
    {
        //testDing1 = 2;
        Serial.println("Unique ID ROOM writed");
    };
};

class CallbacksFrom_MEASURE_INTERVAL_CHAR : public BLECharacteristicCallbacks
{
    void onRead(BLECharacteristic *MEASURE_INTERVAL_CHAR)
    {
        Serial.println("MEASURE_INTERVAL readed");
    };
    void onWrite(BLECharacteristic *MEASURE_INTERVAL_CHAR)
    {
        uint8_t *testDing2;
        testDing2 = MEASURE_INTERVAL_CHAR->getData();

        char test1[50];
        uint8_t counter1 = 0;
        do
        {
            test1[counter1] = testDing2[counter1];
            counter1++;
        } while (test1[counter1 - 1] != 0);
        Serial.println("");
        Serial.print("Readed MEASURE_INTERVAL: ");
        Serial.println(test1);
    };
};

void start_BLE_server()
{
    uint8_t testDing1 = 0;

    uint64_t chipid = ESP.getEfuseMac(); //get ESP32 chip id, as long long unsigned integer
    // Serial.print("unique id: ");
    // Serial.printf("%llu\n", chipid);

#define numberOfUniqueDigits 14 //this is the lenght of chipid, see above

    char BLE_name[12 + numberOfUniqueDigits] = hardcoded_BLE_name; //declare variable for BLE name and add first section of name.
    // Serial.print("BLE_name: ");
    // Serial.println(BLE_name);
    char BLE_unique[numberOfUniqueDigits]; //review this, this step can maybe optimized. declare variable for translation
    sprintf(BLE_unique, "%llu", chipid);   //translate chipid to BLE-unique
    strcat(BLE_name, BLE_unique);          //add BLE_unique to BLE_name
    // Serial.print("BLE_name: ");
    // Serial.println(BLE_name);

    BLEDevice::init(BLE_name); //init Device with builded name char

    BLEServer *pServer = BLEDevice::createServer();

    BLEService *WIFI_PROVISIONING_SERVICE = pServer->createService(WIFI_PROVISIONING_SERVICE_UUID);

    BLECharacteristic *SSID_WIFI_CHAR = WIFI_PROVISIONING_SERVICE->createCharacteristic(
        SSID_WIFI_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    SSID_WIFI_CHAR->setValue("thisWillBeTheLenght"); //review this, needed for memory allocation?

    BLECharacteristic *PASSWORD_WIFI_CHAR = WIFI_PROVISIONING_SERVICE->createCharacteristic(
        PASSWORD_WIFI_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    PASSWORD_WIFI_CHAR->setValue("thisWillBeTheLenght"); //review this, needed for memory allocation?

    BLECharacteristic *STATE_WIFI_CHAR = WIFI_PROVISIONING_SERVICE->createCharacteristic(
        STATE_WIFI_UUID,
        BLECharacteristic::PROPERTY_READ);
    //uint16_t *wifi_char = 0;
    STATE_WIFI_CHAR->setValue(0, 0); //review this, needed for memory allocation?

    BLEService *CONFIG_PROVISIONING_SERVICE = pServer->createService(CONFIG_PROVISIONING_SERVICE_UUID);

    BLECharacteristic *UNIQUE_ID_GATEWAY_CHAR = CONFIG_PROVISIONING_SERVICE->createCharacteristic(
        UNIQUE_ID_GATEWAY_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    //UNIQUE_ID_CHAR->setValue((WiFi.macAddress());
    UNIQUE_ID_GATEWAY_CHAR->setValue("4e:43:a8:f6:73:32"); //see above

    BLECharacteristic *UNIQUE_ID_BOILER_CHAR = CONFIG_PROVISIONING_SERVICE->createCharacteristic(
        UNIQUE_ID_BOILER_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE); //BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE |
    //UNIQUE_ID_CHAR->setValue((WiFi.macAddress());
    UNIQUE_ID_BOILER_CHAR->setValue("4e:43:a8:f6:73:32"); //see above
    //UNIQUE_ID_BOILER_CHAR->addDescriptor(new BLE2902());

    BLECharacteristic *UNIQUE_ID_ROOM_CHAR = CONFIG_PROVISIONING_SERVICE->createCharacteristic(
        UNIQUE_ID_ROOM_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE); //BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE |
    //UNIQUE_ID_CHAR->setValue((WiFi.macAddress());
    UNIQUE_ID_ROOM_CHAR->setValue("4e:43:a8:f6:73:32"); //see above
    //UNIQUE_ID_ROOM_CHAR->addDescriptor(new BLE2902());

    BLECharacteristic *MEASURE_INTERVAL_CHAR = CONFIG_PROVISIONING_SERVICE->createCharacteristic(
        MEASURE_INTERVAL_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    MEASURE_INTERVAL_CHAR->setValue("thisWillBeTheLenght"); //review this, needed for memory allocation?

    WIFI_PROVISIONING_SERVICE->start(); //start service
    CONFIG_PROVISIONING_SERVICE->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising(); // this still is working for backward compatibility

    //BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

    pAdvertising->addServiceUUID(WIFI_PROVISIONING_SERVICE_UUID);   //review this, maybe not required in our case. advertise with argumented service
    pAdvertising->addServiceUUID(CONFIG_PROVISIONING_SERVICE_UUID); //review this, maybe not required in our case. advertise with argumented service

    pAdvertising->setScanResponse(true); //review this?
    pAdvertising->setMinPreferred(0x06); //review this, functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12); //review this?

    BLEDevice::startAdvertising(); //advertise with BLE server settings

    BLEDevice::setPower(ESP_PWR_LVL_N12); //to minimize antenna power

    //BLECharacteristicCallbacks pCharacteristicsCallback = new BLECharacteristicCallbacks();

    SSID_WIFI_CHAR->setCallbacks(new CallbacksFrom_SSID_WIFI_CHAR());         //config callbacks
    PASSWORD_WIFI_CHAR->setCallbacks(new CallbacksFrom_PASSWORD_WIFI_CHAR()); //config callbacks
    STATE_WIFI_CHAR->setCallbacks(new CallbacksFrom_STATE_WIFI_CHAR());       //config callbacks

    UNIQUE_ID_GATEWAY_CHAR->setCallbacks(new CallbacksFrom_UNIQUE_ID_GATEWAY_CHAR()); //config callbacks
    UNIQUE_ID_BOILER_CHAR->setCallbacks(new CallbacksFrom_UNIQUE_ID_BOILER_CHAR());   //config callbacks
    UNIQUE_ID_ROOM_CHAR->setCallbacks(new CallbacksFrom_UNIQUE_ID_ROOM_CHAR());       //config callbacks
    MEASURE_INTERVAL_CHAR->setCallbacks(new CallbacksFrom_MEASURE_INTERVAL_CHAR());   //config callbacks

    boolean wifi_state = false;
    while (1)
    {
        char testVar = Serial.read();
        if (testVar == 'D')
        {
            wifi_state = !wifi_state;
            uint8_t hallo = wifi_state;
            STATE_WIFI_CHAR->setValue(&hallo, 1);
            Serial.printf("ToggledSTATE_WIFI_CHAR to %u", hallo);
        }
        else if (testVar == 'E')
        {
            UNIQUE_ID_BOILER_CHAR->setValue("test1234");
            UNIQUE_ID_BOILER_CHAR->notify();
        }

        if (testDing1 == 1)
        {
            testDing1 = 0;
            UNIQUE_ID_BOILER_CHAR->notify();
        }
        else if (testDing1 == 2)
        {
            testDing1 = 0;
            UNIQUE_ID_ROOM_CHAR->notify();
        }
        delay(10);
    }
}