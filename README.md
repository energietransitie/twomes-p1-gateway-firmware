# Twomes P1 Gateway measurement device
Firmware for the Twomes P1 Gateway measurement device.

## Table of contents
* [General info](#general-info)
* [Deploying](#deploying)
* [Pairing satellites](#pairing-satellites) 
* [Developing](#developing)
* [Features](#features)
* [Status](#status)
* [License](#license)
* [Credits](#credits)

## General info
This repository contains the firmware for the [Twomes P1 Gateway device](https://github.com/energietransitie/twomes-p1-gateway-hardware). It is written in C and is based on the [ESP-IDF](https://github.com/espressif/esp-idf) platform. It uses the [Generic Firmware for Twomes measurement devices](https://github.com/energietransitie/twomes-generic-esp-firmware) for matters such as device preperation, provisioning of home Wi-Fi network credentials, device-backend activation, network time synchronisation via NTP and secure uploading of measurement data. 

This specific firmware reads data from the P1 port of smart meters adhering to the [DSMR standards]((https://www.netbeheernederland.nl/dossiers/slimme-meter-15/documenten). As part of its gateway function, it can be paired with one or more 'satellite' mesurement devices, receive measurements via the energy-efficient [ESP-NOW](https://www.espressif.com/en/products/software/esp-now/overview) protocol, thus lengthening the battery life of such satellite measurement devcies, and upload these messages via Wi-Fi and the internet to a Twomes server.

For the associated hardware design files for the P1 Gateway hardware and enclosure and tips and instructions how to produce and assemble the hardware, please see the [twomes-p1-gateway-hardware](https://github.com/energietransitie/twomes-p1-gateway-hardware) repository. 

## Deploying
This section describes how you can deploy binary releases of the firmware, i.e. without changing the source code, without a development environment and without needing to compile the source code.

### Prerequisites
To deploy the firmware, in addition to the [generic prerequisites for deploying Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#prerequisites), you need:
* a 3.3V TTL-USB Serial Port Adapter (e.g. [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter), CP210x, etc..), including the cable to connect ths adapter to a free USB port on your computer (a USB to miniUSB cable in the case of a [FT232RL](https://www.tinytronics.nl/shop/en/communication-and-signals/usb/ft232rl-3.3v-5v-ttl-usb-serial-port-adapter));
* (optional: more stable) Supply 5V DC power to the device via the micro-USB jack of the device.
* Find a row of 6 holes holes (next to the ESP32 on the PCB of the  P1 Gateway), find the `GND` pin (see  bottom of the PCB), alighn the 6 pins of the serial port adapter such that `GND` and other pins match; then connect the serial port adapter to your computer and connect the 6 pins of the serial port adapter to the 6 holes on the PCB.

### Device preparation step 1: Uploading firmware

* Download the [binary release for your device](https://github.com/energietransitie/twomes-p1-gateway-firmware/releases) and extract it to a directory of your choice.
* If you used the device before, you shoud first [erase all persistenly stored data](https://github.com/energietransitie/twomes-generic-esp-firmware#erasing-all-persistenly-stored-data).
* Follow the [generic Twomes firmware upload instructions ](https://github.com/energietransitie/twomes-generic-esp-firmware#device-preparation-step-1a-uploading-firmware-to-esp32), with the exceptions mentioned below:
	* When you see the beginning of the sequence `Connecting ......_____......`, press and hold the button labeled `GPIO0 (SW2)` on the PCB, then briefly press the button labeled `RESET`, then release the button labeled `GPIO0 (SW2) `;
	* You should see an indication that the firmware is being written to the device.
	* When the upload is finished, view the serial output with a serial monitor tool like [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/) or the utility of your IDE (115200 baud). Press `RESET (SW1)` shortly to  make sure the firmware boots. 


### Device Preparation step 2 and further 
Please follow the [generic firmware instructions for these steps](https://github.com/energietransitie/twomes-generic-esp-firmware#device-preparation-step-2-establishing-a-device-name-and-device-activation_token). 

## Pairing satellites
Pairing a satellite to the  [Twomes P1 Gateway measurement device](https://github.com/energietransitie/twomes-p1-gateway-firmware) works as follows:
* Place the battery in the satellite module, or remove the [insulatting battery pull tab](https://nl.mouser.com/ProductDetail/Keystone-Electronics/117?qs=sGAEpiMZZMv0NwlthflBi4fkPBGNPNEM5y3HK%252B53UQM%3D), make sure it is near the gateway device to be able to see whether the pairing is successful.
* On the satellite module, pressthe `RESET (SW1)` button (labeled `R` on the enclosure); A green LED will start blink for 20 seconds to indicate that the satellite module is in pairing mode, i.e. listens (on 2.4 GHz channel 0) to the gateway device telling it which ESP-NOW channel to use after pairing.
* On the gateway device, within these 20 seconds, press the `GPIO12 (SW2)` (labeled `K` on the enclosure, which stands for the Dutch word "Koppelen"). The gateway device now sends, via 2.4 GHz channel 0, which channel the gateway module should use for subsequent ESP-NOW messages. The green LED on the gateway device blinks shortly during this transmission.
* On the satellite module, when the channel number received, the green LED will turn on steady for about 5 seconds.
* This procedure can be repeated if needed (e.g., when the gateway device is connected to the internet via another Wi-Fi network).

## Developing 
This section describes how you can change the source code using a development environment and compile the source code into a binary release of the firmware that can be depoyed, either via the development environment, or via the method described in the section [Deploying](#deploying).

Please see the [developing section of the generig Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#developing) first. Remember to press buttons to upload the firmware: 
* When you see the beginning of the sequence `Connecting ....___....`, press and hold the button labeled `GPIO0 (SW2)` on the PCB, then briefly press the button labeled `RESET (SW1)`, then release the button labeled `GPIO0 (SW2)`;
* You should see an indication that the firmware is being written to the device.


## Features
List of features ready and TODOs for future development (other than the [features of the generic Twomes firmware](https://github.com/energietransitie/twomes-generic-esp-firmware#features)). 

Ready:
* Read data from the P1 port of devices adhering to DSMRv4 and DSMRv5 (UART settings 115200/8N1).
* Read data from the P1 port of devices adhering to DSMRv2 and DSMRv3 (UART settings 9600/7E1).
* Indicate status and error through LEDs.
* Reset Wi-Fi provisioning by a long press (>10s) of the button `K` (this button is labeled `GPIO12 (SW3)` on the PCB). 
* Confirm pairing with satellite measurement devices with the button `K` (this button is labeled `GPIO12 (SW3)` on the PCB).
* Receive ESP-NOW data from satellite measurement devices and send this to a Twomes server: 
	* [Twomes Boiler Monitor Module](https://github.com/energietransitie/twomes-boiler-monitor-firmware)
	* [Twomes Room Monitor Module](https://github.com/energietransitie/twomes-room-monitor-firmware)  

To-do:

## Status
Project is: in Progress.

## License
This software is available under the [Apache 2.0 license](./LICENSE.md), Copyright 2021 [Research group Energy Transition, Windesheim University of Applied Sciences](https://windesheim.nl/energietransitie) 

## Credits
This software is a collaborative effort of:
* Sjors Smit ·  [@Shorts1999](https://github.com/Shorts1999)

... with help from the following persons for laying the ground work (see legacy branch for their contributions):
* Fredrik-Otto Lautenbag ·  [@Fredrik1997](https://github.com/Fredrik1997)
* Gerwin Buma ·  [@GerwinBuma](https://github.com/GerwinBuma) 
* Werner Heetebrij ·  [@Werner-Heetebrij] (https://github.com/Werner-Heetebrij)
* Henri ter Hofte · [@henriterhofte](https://github.com/henriterhofte) · Twitter [@HeNRGi](https://twitter.com/HeNRGi)

... and with help from the following persons for bugfixes:
* Marco Winkelman · [@MarcoW71](https://github.com/MarcoW71)
* Kevin Janssen · [@KevinJan18](https://github.com/KevinJan18)

We use and gratefully aknowlegde the efforts of the makers of the following source code and libraries:
* [ESP-IDF](https://github.com/espressif/esp-idf), by Espressif Systems, licensed under [Apache 2.0 License](https://github.com/espressif/esp-idf/blob/master/LICENSE)
