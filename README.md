# P1 Poort logger/ gateway
Firmware for ESP32 to read and parse data from Smart meter, receive data from other measurement devices trough ESPNOW, and send all this data to the backoffice trough HTTPS POST

## TODO's and considerations
* ~~Parse P1 data~~
* ~~Use HTTP request to send data to backoffice~~
* ~~Implement ESPNOW for monitor communication~~
* ~~Use https instead of http~~
<<<<<<< HEAD
* ~~Change JSON parameters to the right format~~
* Sync RTC clock with time from smart meter readout
* Add provisioning to exchange the WiFi credentials and unique id trough App
* Add button and LED functionality
* 
=======
* Sync RTC clock with time from smart meter readout/NTP support
* ~~Change JSON parameters to the right format~~
* ~~Add provisioning to exchange the WiFi credentials and unique id trough App~~
* Test provisioning and port to current main software
* Add button functionality
>>>>>>> 76ebfe07750e3e7be2eb9f9670ed1e28db6655db
* Remove debugging code
* send oldest(cached/buffered) data first instead of newest
* improve timing/ system states

## Dependencies
```
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <utils.h>
#include <espnow_settings.h>
#include <ArduinoJson.h>
#include <esp_now.h>
```

## Header files for program settings
The files utils.h.bak and espnow_setting.h.bak contain program enviroment variables. To use the simply remove .bak from the file name and fill in the correct values

## JSON formats Backoffice communication
Smart meter
```
{
    "id": "FF:FF:FF:FF:FF",
    "dataSpec": {
        "lastTime": "201023113050", //=2020/10/23 @ 11:30am 50 sec local time
        "interval": 10,
        "total": 6
    },
    "data": {
        "dsmr": [
            50
        ],
        "evt1": [
            2955336
        ],
        "evt2": [
            3403620
        ],
        "egt1": [
            2
        ],
        "egt2": [
            0
        ],
        "ht": [
            2
        ],
        "ehv": [
            349
        ],
        "ehl": [
            0
        ],
        "gas": [
            2478797
        ],
        "tgas": [
            201023113008
        ]
    }
}

```

room temperature monitor
```
{
    "id": "FF:FF:FF:FF:FF",
    "dataSpec": {
        "lastTime": 1606915033, //=12/02/2020 @ 1:17pm (UTC)
        "interval": 10,
        "total": 6
    },
    "data": {
        "roomTemp": [
            20.1,
            21.2,
            22.3,
            23.4,
            24.5,
            25.6
        ]
    }
}

```

Boiler temperature monitor
```
{
    "id": "FF:FF:FF:FF:FF",
    "dataSpec": {
        "lastTime": 1606914671, //=12/02/2020 @ 1:11pm (UTC)
        "interval": 10,
        "total": 6
    },
    "data": {
        "pipeTemp1": [ //this should be the 'hot' outgoing pipe
            50.1,
            51.2,
            52.3,
            53.4,
            54.5
        ],
        "pipeTemp2": [ //this should be the 'cold' incoming pipe
            50.1,
            51.2,
            52.3,
            53.4,
            54.5
        ]
    }
}

```
<<<<<<< HEAD
=======

## Dependencies
```
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <utils.h>
#include <espnow_settings.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <BLE.h>
```
>>>>>>> 76ebfe07750e3e7be2eb9f9670ed1e28db6655db
