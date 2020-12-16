# P1 Poort logger
Firmware for ESP32 to read and parse data from Smart meter and send to backoffice trough HTTP POST

## TODO's and considerations
* ~~Parse P1 data~~
* ~~Use HTTP request to send data to backoffice~~
* Implement ESPNOW for monitor communication
* Remove HTTPClient library and make HTTP request with WiFi library
* Use https instead of http
* Sync RTC clock with time from smart meter readout
* Change JSON parameters to the right format
* Add provisioning to exchange the WiFi credentials and unique id trough App
* Add button functionality
* Remove debugging code

## JSON formats Backoffice communication
Smart meter
```
{
    "id": "FF:FF:FF:FF:FF",
    "dataSpec": {
        "lastTime": "201023113050",
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

## Dependencies
```
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <utils.h>
```