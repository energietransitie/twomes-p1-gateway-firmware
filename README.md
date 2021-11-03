# Twomes P1 Gateway measurement device
Firmware for the Twomes P1 Gateway measurement device.

## Table of contents
* [General info](#general-info)
* [Deploying](#deploying)
* [Developing](#developing) 
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
This firmware is designed to run on the ESP32 of the Twomes P1 Gateway device. It is written using C and ESP-IDF. It uses the [Generic Firmware for Twomes measuremetn devices](https://github.com/energietransitie/twomes-generic-esp-firmware) for secure HTTPS POST to the Twomes Backoffice, Twomes "Warmtewachter" provisioning and NTP timestamping.
The firmware can read data from DSMR4 or DSMR5 Smart Energy meters and it can receive ESP-Now messages from various Twomes "Satellites".

For the associated hardware design files for the P1 Gateway hardware and enclosure and tips and instructions how to produce and assemble the hardware, please see the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware) repository. 

## Deploying
This section describes how you can deploy binary releases of the firmware, i.e. without changing the source code, without a development environment and without needing to compile the source code.

### Prerequisites
To deploy the firmware, in addition to the [generic prerequisites for Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#prerequisites), you need:
* a 3.3V TTL-USB Serial Port Adapter (e.g. [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter) CP210x, etc..), including the cable to connect ths adapter to a free USB port on your computer (a USB to miniUSB cable in the case of a  [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter));

### Uploading firmware

* Download the [binary release for your device](https://github.com/energietransitie/twomes-p1-gateway-firmware/releases) and extract it to a directory of your choice.
* If you used the device before, you shoud first [erase all persistenly stored data](#erasing-all-persistenly-stored-data).
* (optional: more stable) Supply 5V DC power to the device via the micro-USB jack of the device.
* Before you connect the serial port adapter to your computer and connect the 6 pins of the serial port adapter to the  6 holes next to the ESP32 on the P1-Gateway PCB; then connect the 6 pins to the 6 holes.
* Open a comand prompt in that directory, change the directory to the binaries subfolder and enter:
	```shell
	py -m esptool --chip esp32 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x9000 partitions.bin 0xe000 ota_data_initial.bin 0x10000 firmware.bin  
	```
* This should automatically detect the USB port that the device is connected to.
* If not, then open the Device Manager (in Windows press the `Windows + X` key combination, then select Device Manager), go to View and click Show Hidden Devices. Then unfold `Ports (COM & LPT)`. You should find the device there, named `USB-Serial CH340 *(COM?)` with `?` being a single digit.  
* If the COM port is not automatically detected, then enter (while replacing `?` with the digit found in the previous step): 
	```shell
	py -m esptool --chip esp32 --port "COM?" --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader.bin 0x9000 partitions.bin 0xe000 ota_data_initial.bin 0x10000 firmware.bin```

* When you see the beginning of the sequence `conecting ....___....`, press and hold the button labeled `GPIO1` on the PCB, then briefly press the button labeled `RESET`,then release the button labeled `GPIO1`;
* You should see an indication that the firmware is being written to the device.
* Proceed with device preparation like a generic Twomes device.

## Developing 
This section describes how you can change the source code using a development environment and compile the source code into a binary release of the firmware that can be depoyed, either via the development environment, or via the method described in the section [Deploying](#deploying).

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
