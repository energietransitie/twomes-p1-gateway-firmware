# Twomes P1 Gateway measurement device
Firmware for the Twomes P1 Gateway measurement device.

## Table of contents
* [General info](#general-info)
* [Using binary releases](#using-binaries-releases)
* [Developing with the source code ](#developing-with-the-source-code) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
This firmware is designed to run on the ESP32 of the Twomes P1 Gateway device. It is written using C and ESP-IDF. It uses the [Generic Firmware for Twomes measuremetn devices](https://github.com/energietransitie/twomes-generic-esp-firmware) for secure HTTPS POST to the Twomes Backoffice, Twomes "Warmtewachter" provisioning and NTP timestamping.
The firmware can read data from DSMR4 or DSMR5 Smart Energy meters and it can receive ESP-Now messages from various Twomes "Satellites".

For the associated hardware design files for the P1 Gateway hardware and enclosure and tips and instructions how to produce and assemble the hardware, please see the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware) repository. 

## Using binary releases
### Prerequisites
*	a device based on an ESP32 SoC, such as the [LilyGO TTGO T7 Mini32 V1.3 ESP32](https://github.com/LilyGO/ESP32-MINI-32-V1.3) or an ESP8266 SoC, such as the [Wemos LOLIN D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html);
*	a USB to micro-USB cable;
*	a PC with a USB port;
*	[Python v3.8 or above](https://www.python.org/downloads/) installed, and make sure to select `Add Python <version number> to PATH` so you can use the Python commands we document below from a command prompt;
*	[Esptool](https://github.com/espressif/esptool) installed, the Espressif SoC serial bootloader utility;
*	[PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/), a serial monitor utility. (If your you're also developing, you may use the serial monitor utility in your IDE, instead).

###how to upload firmware

Connect a USB<->Serial converter to the 6 holes next to the ESP32 on the P1 Gateway, the pinout is described on the bottom of the device. 
(optional) Connect the USB power port on the P1 Gateway to supply it with power, this is more stable than the power supplied through the programmer

###On windows:
Run UPLOAD.bat 
###On Linux/Mac
Run the following command on the _command line_:
	```shell
	py -m esptool --chip esp32 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 ota_data_initial.bin 0x10000 firmware.bin  
	```
*	This should automatically detect the USB port that the device is connected to.
*	If not, then open the Device Manager (in Windows press the `Windows + X` key combination, then select Device Manager), go to View and click Show Hidden Devices. Then unfold `Ports (COM & LPT)`. You should find the device there, named `USB-Serial CH340 *(COM?)` with `?` being a single digit.  
*	If the COM port is not automatically detected, then enter (while replacing `?` with the digit found in the previous step): 
	```shell
	py -m esptool --chip esp32 --port "COM?" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x8000 partitions.bin 0xe000 ota_data_initial.bin 0x10000 firmware.bin
	```

## Developing with the source code 
Install [Visual Studio Code](https://code.visualstudio.com/) and the [PlatformIO](https://platformio.org/platformio-ide) plugin

Download the sourcecode, unzip it, and open the folder in Visual Studio Code.
Modify the sourcecode to fit your needs. Connect your board, and press the arrow on the blue bar on the bottom left to compile and upload your code

## Features
List of features ready and TODOs for future development. 

Ready:
* Receive ESP-Now data from sensors
* Package received data into JSON
* Receive Network Credentials through SoftAP unified provisioning
* indicate status and error through LEDs
* Receive user input through buttons
* Reset provisioning when P2 (GPIO 12) is held for over 10 seconds
* Implement Channel and MAC provisioning to sensor nodes
* Implement P1 reading and packaging
* HTTPS post to backoffice
* 
To-do:
* Update for CO₂ sensor data
* Test with Room sensor data
* buffer data when transmission fails

## Status
Project is: in Progress.
In its current state the software can properly receive temperatures and package them into the JSON format for twomes. The device can also read the P1 port and send the necessary data to the Twomes backoffice.
There are still some missing features that need to be implemented, see [To-do](#features) for more info

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software is a collaborative effort of:
* Sjors Smit ·  [@Shorts1999](https://github.com/Shorts1999)

... with help from the following persons for laying the ground work (see legacy branch for their contributions):
* Fredrik-Otto Lautenbag ·  [@Fredrik1997](https://github.com/Fredrik1997)
* Gerwin Buma ·  [@GerwinBuma](https://github.com/GerwinBuma) 
* Werner Heetebrij ·  [@Werner-Heetebrij] (https://github.com/Werner-Heetebrij)

... and with help from the following persons for bugfixes:
* Marco Winkelman · [@MarcoW71](https://github.com/MarcoW71)
* Kevin Janssen · [@KevinJan18](https://github.com/KevinJan18)

We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache 2.0 License](https://github.com/espressif/esp-idf/blob/master/LICENSE)
