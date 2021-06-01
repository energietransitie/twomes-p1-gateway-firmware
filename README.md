# Twomes Boiler Temperature Sensor
Firmware for the Twomes "Digital Twins for the Heat-transition" P1 port Gateway

## Table of contents
* [General info](#general-info)
* [Using binary releases](#using-binaries-releases)
* [Developing with the source code ](#developing-with-the-source-code) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
This firmware is designed to run on the ESP32 of the Twomes P1 Gateway device. It is written using C and ESP-IDF. It uses the Generic Twomes Firmware for secure HTTPS POST to the Twomes Backoffice, Twomes "Warmtewachter" provisioning and NTP timestamping.
The firmware can read data from DSMR4 or DSMR5 Smart Energy meters and it can receive ESP-Now messages from various Twomes "Satellites".

## Using binary releases
You can download and locally install the lastest installable version(s) via <link to the latest binary release(s) you published and describe to how people can install and run thise binaries; if needed describe this for different platforms>.

## Developing with the source code 
Describe how to use / adapt/ compile the souce code. 

## Features
List of features ready and TODOs for future development. 

Ready:
* Receive ESP-Now data from sensors
* Package received data into JSON
* Receive Network Credentials through BLE unified provisioning
* indicate status and error through LEDs
* Receive user input through buttons
* Reset provisioning when P2 (GPIO 12) is held for over 10 seconds
* Read P1 port and package into struct

To-do:
* Implement Channel and MAC provisioning to sensor nodes
* Package P1 read data into JSON
* HTTPS post to backoffice

## Status
Project is: in Progress.
In its current state the software can properly receive temperatures and package them into the JSON format for twomes.
There are still some missing features that need to be implemented, see [To-do](#features) for more info

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software is a collaborative effort the following students and researchers:
* Fredrik-Otto Lautenbag ·  [@Fredrik1997](https://github.com/Fredrik1997)
* Gerwin Buma ·  [@GerwinBuma](https://github.com/GerwinBuma) 
* Sjors Smit ·  [@Shorts1999](https://github.com/Shorts1999)
* Marco Winkelman · [@MarcoW71](https://github.com/MarcoW71)


We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:

* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache 2.0 License](https://github.com/espressif/esp-idf/blob/master/LICENSE)

## Contact
<not yet determined; which contact info to include here?>
