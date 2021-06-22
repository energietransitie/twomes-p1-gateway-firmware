EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Twomes P1 Gateway"
Date "2021-04-12"
Rev "1"
Comp "Lectoraat Energietransitie"
Comment1 "Author: Sjors Smit"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RF_Module:ESP32-WROOM-32D U2
U 1 1 607413A7
P 8150 2650
F 0 "U2" H 7700 4000 50  0000 C CNN
F 1 "ESP32-WROOM-32D" H 8650 4000 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 8150 1150 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf" H 7850 2700 50  0001 C CNN
	1    8150 2650
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x06_Female J1
U 1 1 607438A6
P 1200 6450
F 0 "J1" H 1092 6835 50  0000 C CNN
F 1 "FTDI header" H 1250 6750 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 1200 6450 50  0001 C CNN
F 3 "~" H 1200 6450 50  0001 C CNN
	1    1200 6450
	-1   0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 60746B01
P 2450 4400
F 0 "C2" H 2565 4446 50  0000 L CNN
F 1 "1uF" H 2500 4300 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2488 4250 50  0001 C CNN
F 3 "~" H 2450 4400 50  0001 C CNN
	1    2450 4400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 60746F17
P 2750 4400
F 0 "C3" H 2865 4446 50  0000 L CNN
F 1 "100nF" H 2800 4300 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2788 4250 50  0001 C CNN
F 3 "~" H 2750 4400 50  0001 C CNN
	1    2750 4400
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 607473A5
P 1100 4400
F 0 "C1" H 1215 4446 50  0000 L CNN
F 1 "1uF" H 1150 4300 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1138 4250 50  0001 C CNN
F 3 "~" H 1100 4400 50  0001 C CNN
	1    1100 4400
	1    0    0    -1  
$EndComp
Connection ~ 2100 4800
Connection ~ 2450 4250
Connection ~ 2750 4250
Wire Wire Line
	2750 4250 2450 4250
Wire Wire Line
	2750 4250 2850 4250
$Comp
L power:+3.3V #PWR06
U 1 1 6074A024
P 2850 4250
F 0 "#PWR06" H 2850 4100 50  0001 C CNN
F 1 "+3.3V" H 2865 4423 50  0000 C CNN
F 2 "" H 2850 4250 50  0001 C CNN
F 3 "" H 2850 4250 50  0001 C CNN
	1    2850 4250
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR03
U 1 1 6074A660
P 1500 1200
F 0 "#PWR03" H 1500 1050 50  0001 C CNN
F 1 "+5V" H 1515 1373 50  0000 C CNN
F 2 "" H 1500 1200 50  0001 C CNN
F 3 "" H 1500 1200 50  0001 C CNN
	1    1500 1200
	1    0    0    -1  
$EndComp
Text Label 9150 2450 2    50   ~ 0
RX_P1
Text Label 9350 2550 2    50   ~ 0
ESP_DATA_REQ
Wire Wire Line
	8750 1550 9150 1550
Text Label 9150 1550 2    50   ~ 0
ESP_TX
Text Label 9150 1750 2    50   ~ 0
ESP_RX
Wire Wire Line
	7550 1450 7050 1450
Text Label 7050 1450 0    50   ~ 0
RESET
$Comp
L power:+3.3V #PWR07
U 1 1 6075968E
P 8150 1200
F 0 "#PWR07" H 8150 1050 50  0001 C CNN
F 1 "+3.3V" H 8165 1373 50  0000 C CNN
F 2 "" H 8150 1200 50  0001 C CNN
F 3 "" H 8150 1200 50  0001 C CNN
	1    8150 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8150 1250 8150 1200
$Comp
L power:GND #PWR08
U 1 1 60759FFF
P 8150 4100
F 0 "#PWR08" H 8150 3850 50  0001 C CNN
F 1 "GND" H 8155 3927 50  0000 C CNN
F 2 "" H 8150 4100 50  0001 C CNN
F 3 "" H 8150 4100 50  0001 C CNN
	1    8150 4100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 6075A10B
P 2100 4850
F 0 "#PWR05" H 2100 4600 50  0001 C CNN
F 1 "GND" H 2105 4677 50  0000 C CNN
F 2 "" H 2100 4850 50  0001 C CNN
F 3 "" H 2100 4850 50  0001 C CNN
	1    2100 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 4850 2100 4800
Wire Wire Line
	1500 1200 1500 1300
Text Label 1800 1400 2    50   ~ 0
DATA_REQ
Text Label 1800 1700 2    50   ~ 0
RX_P1
Wire Wire Line
	1900 1500 1900 1800
Wire Wire Line
	1400 1500 1900 1500
Wire Wire Line
	1500 1300 1400 1300
Wire Wire Line
	1650 1800 1650 1900
$Comp
L power:GND #PWR04
U 1 1 6075C3A7
P 1650 1900
F 0 "#PWR04" H 1650 1650 50  0001 C CNN
F 1 "GND" H 1655 1727 50  0000 C CNN
F 2 "" H 1650 1900 50  0001 C CNN
F 3 "" H 1650 1900 50  0001 C CNN
	1    1650 1900
	1    0    0    -1  
$EndComp
$Comp
L Connector:RJ12 J2
U 1 1 60741E37
P 1000 1500
F 0 "J2" H 1057 2067 50  0000 C CNN
F 1 "RJ12-6P6C" H 1057 1976 50  0000 C CNN
F 2 "Connector_RJ:RJ12_Amphenol_54601" V 1000 1525 50  0001 C CNN
F 3 "~" V 1000 1525 50  0001 C CNN
	1    1000 1500
	1    0    0    1   
$EndComp
Wire Wire Line
	1400 1800 1650 1800
NoConn ~ 1400 1600
Connection ~ 1650 1800
Wire Wire Line
	1650 1800 1900 1800
Wire Wire Line
	1100 3250 1100 3400
$Comp
L Device:R R2
U 1 1 6076D1AE
P 1100 3100
F 0 "R2" H 1170 3146 50  0000 L CNN
F 1 "1R" H 1170 3055 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 1030 3100 50  0001 C CNN
F 3 "~" H 1100 3100 50  0001 C CNN
	1    1100 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 6076ED2A
P 800 3800
F 0 "R1" H 730 3754 50  0000 R CNN
F 1 "1k" H 730 3845 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 730 3800 50  0001 C CNN
F 3 "~" H 800 3800 50  0001 C CNN
	1    800  3800
	1    0    0    1   
$EndComp
Wire Wire Line
	800  3950 800  4000
Wire Wire Line
	800  2850 950  2850
Wire Wire Line
	800  2850 800  3050
$Comp
L power:+5V #PWR02
U 1 1 6077493C
P 950 2800
F 0 "#PWR02" H 950 2650 50  0001 C CNN
F 1 "+5V" H 965 2973 50  0000 C CNN
F 2 "" H 950 2800 50  0001 C CNN
F 3 "" H 950 2800 50  0001 C CNN
	1    950  2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	950  2800 950  2850
Connection ~ 950  2850
Wire Wire Line
	8150 4050 8150 4100
$Comp
L power:GND #PWR01
U 1 1 6077B58D
P 800 4000
F 0 "#PWR01" H 800 3750 50  0001 C CNN
F 1 "GND" H 805 3827 50  0000 C CNN
F 2 "" H 800 4000 50  0001 C CNN
F 3 "" H 800 4000 50  0001 C CNN
	1    800  4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2750 4550 2750 4800
Wire Wire Line
	2450 4550 2450 4800
Connection ~ 2450 4800
Wire Wire Line
	2450 4800 2750 4800
Wire Wire Line
	1100 4550 1100 4800
Text Notes 1600 2750 0    79   ~ 16
Current limit\n& Voltage regulation
Wire Wire Line
	1100 3800 1100 4250
Text Notes 750  1000 0    79   ~ 16
P1 Port
Wire Notes Line
	3100 400  3100 7900
Text Notes 1700 5650 0    79   ~ 16
Programmer interface
$Comp
L TwomesGateway-rescue:AP2112K-3.3-Regulator_Linear U1
U 1 1 6077D110
P 2000 4350
F 0 "U1" H 2000 4692 50  0000 C CNN
F 1 "AP7365-33WG-7" H 2100 4600 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 2000 4675 50  0001 C CNN
F 3 "https://www.diodes.com/assets/Datasheets/AP2112.pdf" H 2000 4450 50  0001 C CNN
	1    2000 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 4350 1650 4350
Wire Wire Line
	1650 4350 1650 4250
Connection ~ 1650 4250
Wire Wire Line
	1650 4250 1700 4250
Wire Wire Line
	2000 4650 2000 4800
Connection ~ 2000 4800
Wire Wire Line
	2000 4800 2100 4800
Wire Wire Line
	2300 4250 2450 4250
Wire Wire Line
	2100 4800 2450 4800
NoConn ~ 1900 6250
NoConn ~ 1900 6650
Text Label 1800 6650 2    50   ~ 0
FTDI_CTS
Text Label 1800 6450 2    50   ~ 0
RX0_5V
Text Label 1800 6350 2    50   ~ 0
ESP_TX
Wire Wire Line
	1400 6250 1900 6250
Wire Wire Line
	1400 6650 1900 6650
Wire Wire Line
	1400 6750 1900 6750
$Comp
L power:GND #PWR09
U 1 1 60787086
P 1900 6800
F 0 "#PWR09" H 1900 6550 50  0001 C CNN
F 1 "GND" H 1905 6627 50  0000 C CNN
F 2 "" H 1900 6800 50  0001 C CNN
F 3 "" H 1900 6800 50  0001 C CNN
	1    1900 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 6800 1900 6750
Text Label 1800 6250 2    50   ~ 0
FTDI_DTR
Wire Wire Line
	1800 6450 1400 6450
Wire Wire Line
	1400 6350 1800 6350
Wire Wire Line
	2300 6500 2300 6550
Wire Wire Line
	1400 6550 2300 6550
$Comp
L Diode:1N4148W D1
U 1 1 607909D4
P 1650 7500
F 0 "D1" H 1650 7717 50  0000 C CNN
F 1 "1N4148W" H 1650 7626 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-123" H 1650 7325 50  0001 C CNN
F 3 "https://www.vishay.com/docs/85748/1n4148w.pdf" H 1650 7500 50  0001 C CNN
	1    1650 7500
	1    0    0    -1  
$EndComp
Text Label 1150 7500 0    50   ~ 0
RX0_5V
Wire Wire Line
	1150 7500 1500 7500
Text Label 2100 7500 2    50   ~ 0
ESP_RX
Wire Wire Line
	2100 7500 1800 7500
Text Notes 2150 7650 2    50   ~ 0
For 5V programmer support
$Comp
L Device:R R4
U 1 1 6079C456
P 4600 2100
F 0 "R4" V 4807 2100 50  0000 C CNN
F 1 "2.2k" V 4716 2100 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 4530 2100 50  0001 C CNN
F 3 "~" H 4600 2100 50  0001 C CNN
	1    4600 2100
	0    1    -1   0   
$EndComp
$Comp
L power:+5V #PWR011
U 1 1 607A0539
P 3900 1150
F 0 "#PWR011" H 3900 1000 50  0001 C CNN
F 1 "+5V" H 3915 1323 50  0000 C CNN
F 2 "" H 3900 1150 50  0001 C CNN
F 3 "" H 3900 1150 50  0001 C CNN
	1    3900 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 1550 4300 1550
Wire Wire Line
	3900 1150 3900 1350
Wire Wire Line
	3900 1750 3900 2000
Text Label 3450 2000 0    50   ~ 0
DATA_REQ
Wire Wire Line
	3450 2000 3900 2000
Wire Wire Line
	8750 2550 9350 2550
Text Label 5350 2100 2    50   ~ 0
ESP_DATA_REQ
Wire Wire Line
	4750 2100 5350 2100
$Comp
L power:+3.3V #PWR013
U 1 1 607B24B8
P 9500 1950
F 0 "#PWR013" H 9500 1800 50  0001 C CNN
F 1 "+3.3V" H 9515 2123 50  0000 C CNN
F 2 "" H 9500 1950 50  0001 C CNN
F 3 "" H 9500 1950 50  0001 C CNN
	1    9500 1950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 607B6EDC
P 9500 2150
F 0 "R5" H 9570 2196 50  0000 L CNN
F 1 "10k" H 9570 2105 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 9430 2150 50  0001 C CNN
F 3 "~" H 9500 2150 50  0001 C CNN
	1    9500 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	9500 2000 9500 1950
Wire Wire Line
	9500 2300 9500 2450
Wire Wire Line
	8750 2450 9500 2450
$Comp
L TwomesGateway-rescue:USB_B_Micro-Connector J3
U 1 1 607C9555
P 3500 3900
F 0 "J3" H 3557 4367 50  0000 C CNN
F 1 "USB_B_Micro" H 3557 4276 50  0000 C CNN
F 2 "TwomesGateway:ÙSB_JLC__SHOUHAN_microQTJ" H 3650 3850 50  0001 C CNN
F 3 "~" H 3650 3850 50  0001 C CNN
	1    3500 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 4300 3450 4300
NoConn ~ 3800 3900
NoConn ~ 3800 4000
NoConn ~ 3800 4100
Wire Wire Line
	3800 3700 3950 3700
Wire Wire Line
	3950 3700 3950 3650
$Comp
L power:VBUS #PWR016
U 1 1 607D026B
P 3950 3650
F 0 "#PWR016" H 3950 3500 50  0001 C CNN
F 1 "VBUS" H 3965 3823 50  0000 C CNN
F 2 "" H 3950 3650 50  0001 C CNN
F 3 "" H 3950 3650 50  0001 C CNN
	1    3950 3650
	1    0    0    -1  
$EndComp
$Comp
L power:VBUS #PWR014
U 1 1 607D716C
P 2300 6500
F 0 "#PWR014" H 2300 6350 50  0001 C CNN
F 1 "VBUS" H 2315 6673 50  0000 C CNN
F 2 "" H 2300 6500 50  0001 C CNN
F 3 "" H 2300 6500 50  0001 C CNN
	1    2300 6500
	1    0    0    -1  
$EndComp
$Comp
L power:VBUS #PWR010
U 1 1 607D7835
P 1600 3850
F 0 "#PWR010" H 1600 3700 50  0001 C CNN
F 1 "VBUS" H 1750 3950 50  0000 C CNN
F 2 "" H 1600 3850 50  0001 C CNN
F 3 "" H 1600 3850 50  0001 C CNN
	1    1600 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 4250 1600 4250
Connection ~ 1100 4250
Wire Wire Line
	1100 4800 2000 4800
Wire Wire Line
	1600 3850 1600 3900
Wire Wire Line
	1600 4200 1600 4250
Connection ~ 1600 4250
Wire Wire Line
	1600 4250 1650 4250
$Comp
L power:GND #PWR015
U 1 1 607E8728
P 3450 4300
F 0 "#PWR015" H 3450 4050 50  0001 C CNN
F 1 "GND" H 3455 4127 50  0000 C CNN
F 2 "" H 3450 4300 50  0001 C CNN
F 3 "" H 3450 4300 50  0001 C CNN
	1    3450 4300
	1    0    0    -1  
$EndComp
Connection ~ 3450 4300
Wire Wire Line
	3450 4300 3500 4300
Wire Notes Line
	4500 2350 4500 5350
Text Notes 4200 2750 2    79   ~ 16
Additional USB\npower supply
Text Notes 4900 700  2    79   ~ 16
P1 Data request level shift
Wire Notes Line
	5750 2350 5750 400 
Wire Notes Line
	400  2350 5750 2350
Text Notes 10450 2000 2    50   ~ 0
Open collector Pull-up
Text Notes 7750 1000 2    79   ~ 16
ESP32 Microcontroller
NoConn ~ 8750 1850
NoConn ~ 8750 1950
NoConn ~ 8750 2350
NoConn ~ 8750 2650
NoConn ~ 8750 2750
NoConn ~ 8750 2850
NoConn ~ 8750 2950
NoConn ~ 8750 3050
NoConn ~ 8750 3150
NoConn ~ 8750 3250
NoConn ~ 8750 3350
NoConn ~ 8750 3450
NoConn ~ 8750 3550
NoConn ~ 8750 3650
NoConn ~ 8750 3750
NoConn ~ 7550 3150
NoConn ~ 7550 3050
NoConn ~ 7550 2950
NoConn ~ 7550 2850
NoConn ~ 7550 2750
NoConn ~ 7550 2650
$Comp
L Device:R R6
U 1 1 60816629
P 3550 6450
F 0 "R6" H 3480 6404 50  0000 R CNN
F 1 "10k" H 3480 6495 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3480 6450 50  0001 C CNN
F 3 "~" H 3550 6450 50  0001 C CNN
	1    3550 6450
	1    0    0    1   
$EndComp
$Comp
L Device:LED D3
U 1 1 60816EF9
P 5750 7250
F 0 "D3" V 5789 7132 50  0000 R CNN
F 1 "LED_RED" V 5698 7132 50  0000 R CNN
F 2 "LED_SMD:LED_1206_3216Metric" H 5750 7250 50  0001 C CNN
F 3 "~" H 5750 7250 50  0001 C CNN
	1    5750 7250
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED D4
U 1 1 60817743
P 6250 7250
F 0 "D4" V 6289 7132 50  0000 R CNN
F 1 "LED_GREEN" V 6198 7132 50  0000 R CNN
F 2 "Diode_SMD:D_0805_2012Metric" H 6250 7250 50  0001 C CNN
F 3 "~" H 6250 7250 50  0001 C CNN
	1    6250 7250
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 60817A48
P 3550 6900
F 0 "SW1" V 3600 7200 50  0000 R CNN
F 1 "SW_Push" V 3500 7300 50  0000 R CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_6x6mm_H9.5mm" H 3550 7100 50  0001 C CNN
F 3 "~" H 3550 7100 50  0001 C CNN
	1    3550 6900
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 6081825E
P 4500 6950
F 0 "SW2" V 4546 6902 50  0000 R CNN
F 1 "SW_Push" V 4455 6902 50  0000 R CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_6x6mm_H9.5mm" H 4500 7150 50  0001 C CNN
F 3 "~" H 4500 7150 50  0001 C CNN
	1    4500 6950
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW3
U 1 1 60818509
P 4850 6950
F 0 "SW3" V 4896 6902 50  0000 R CNN
F 1 "SW_Push" V 4805 6902 50  0000 R CNN
F 2 "Button_Switch_SMD:SW_Push_1P1T_NO_6x6mm_H9.5mm" H 4850 7150 50  0001 C CNN
F 3 "~" H 4850 7150 50  0001 C CNN
	1    4850 6950
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R8
U 1 1 608187AF
P 6250 6850
F 0 "R8" H 6180 6804 50  0000 R CNN
F 1 "1k" H 6180 6895 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 6180 6850 50  0001 C CNN
F 3 "~" H 6250 6850 50  0001 C CNN
	1    6250 6850
	1    0    0    1   
$EndComp
$Comp
L Device:R R7
U 1 1 60818ABB
P 5750 6850
F 0 "R7" H 5680 6804 50  0000 R CNN
F 1 "1k" H 5680 6895 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5680 6850 50  0001 C CNN
F 3 "~" H 5750 6850 50  0001 C CNN
	1    5750 6850
	1    0    0    1   
$EndComp
Wire Wire Line
	6250 7000 6250 7100
Wire Wire Line
	5750 7000 5750 7100
$Comp
L power:GND #PWR0101
U 1 1 608211A2
P 5750 7450
F 0 "#PWR0101" H 5750 7200 50  0001 C CNN
F 1 "GND" H 5755 7277 50  0000 C CNN
F 2 "" H 5750 7450 50  0001 C CNN
F 3 "" H 5750 7450 50  0001 C CNN
	1    5750 7450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 6082173B
P 6250 7450
F 0 "#PWR0102" H 6250 7200 50  0001 C CNN
F 1 "GND" H 6255 7277 50  0000 C CNN
F 2 "" H 6250 7450 50  0001 C CNN
F 3 "" H 6250 7450 50  0001 C CNN
	1    6250 7450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6250 7450 6250 7400
Wire Wire Line
	5750 7400 5750 7450
Wire Wire Line
	6250 6700 6250 6450
Wire Wire Line
	5750 6700 5750 6450
Text Label 5950 6450 2    50   ~ 0
LED_ERROR
Text Label 6550 6450 2    50   ~ 0
LED_STATUS
Wire Wire Line
	6250 6450 6550 6450
Wire Wire Line
	5750 6450 5950 6450
Wire Wire Line
	4850 7150 4850 7250
Wire Wire Line
	4850 7250 4500 7250
Wire Wire Line
	3550 7250 3550 7100
Wire Wire Line
	4500 7150 4500 7250
Connection ~ 4500 7250
Wire Wire Line
	4500 7250 4200 7250
Wire Wire Line
	3550 6600 3550 6650
Wire Wire Line
	3550 6300 3550 6150
Wire Wire Line
	3550 6650 3850 6650
Connection ~ 3550 6650
Wire Wire Line
	3550 6650 3550 6700
$Comp
L Device:C C4
U 1 1 6083A492
P 3850 6950
F 0 "C4" H 3965 6996 50  0000 L CNN
F 1 "100nF" H 3965 6905 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3888 6800 50  0001 C CNN
F 3 "~" H 3850 6950 50  0001 C CNN
	1    3850 6950
	1    0    0    -1  
$EndComp
Wire Wire Line
	3850 6800 3850 6650
Wire Wire Line
	3850 7100 3850 7250
Connection ~ 3850 7250
Wire Wire Line
	3850 7250 3550 7250
Wire Wire Line
	4500 6750 4500 6500
Wire Wire Line
	4850 6750 4850 6500
Wire Wire Line
	4850 6500 4600 6500
Text Label 4600 6500 0    50   ~ 0
GPIO12
Text Label 4300 6500 0    50   ~ 0
GPIO0
Wire Wire Line
	4500 6500 4300 6500
$Comp
L power:+3.3V #PWR0103
U 1 1 608480BD
P 3550 6150
F 0 "#PWR0103" H 3550 6000 50  0001 C CNN
F 1 "+3.3V" H 3565 6323 50  0000 C CNN
F 2 "" H 3550 6150 50  0001 C CNN
F 3 "" H 3550 6150 50  0001 C CNN
	1    3550 6150
	1    0    0    -1  
$EndComp
Text Label 3850 6650 2    50   ~ 0
RESET
Wire Wire Line
	8750 1450 9150 1450
Text Label 9150 1450 2    50   ~ 0
GPIO0
Text Label 9150 2050 2    50   ~ 0
GPIO12
Wire Wire Line
	8750 2050 9150 2050
Text Label 9200 2150 2    50   ~ 0
LED_ERROR
Text Label 9250 2250 2    50   ~ 0
LED_STATUS
Wire Wire Line
	8750 2250 9250 2250
Wire Wire Line
	8750 2150 9200 2150
Wire Wire Line
	9150 1750 8750 1750
Wire Notes Line
	6950 5350 6950 7900
Wire Notes Line
	400  5350 6950 5350
Text Notes 6600 5600 2    79   ~ 16
User I/O
NoConn ~ 7550 1650
NoConn ~ 7550 1750
NoConn ~ 8750 1650
$Comp
L Device:C C5
U 1 1 6087C210
P 9850 1150
F 0 "C5" H 9965 1196 50  0000 L CNN
F 1 "100nF" H 9950 1100 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9888 1000 50  0001 C CNN
F 3 "~" H 9850 1150 50  0001 C CNN
	1    9850 1150
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0104
U 1 1 60882965
P 10050 1000
F 0 "#PWR0104" H 10050 850 50  0001 C CNN
F 1 "+3.3V" H 10065 1173 50  0000 C CNN
F 2 "" H 10050 1000 50  0001 C CNN
F 3 "" H 10050 1000 50  0001 C CNN
	1    10050 1000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0105
U 1 1 60882D96
P 10050 1300
F 0 "#PWR0105" H 10050 1050 50  0001 C CNN
F 1 "GND" H 10055 1127 50  0000 C CNN
F 2 "" H 10050 1300 50  0001 C CNN
F 3 "" H 10050 1300 50  0001 C CNN
	1    10050 1300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 608E2FA5
P 4200 7250
F 0 "#PWR0106" H 4200 7000 50  0001 C CNN
F 1 "GND" H 4205 7077 50  0000 C CNN
F 2 "" H 4200 7250 50  0001 C CNN
F 3 "" H 4200 7250 50  0001 C CNN
	1    4200 7250
	1    0    0    -1  
$EndComp
Connection ~ 4200 7250
Wire Wire Line
	4200 7250 3850 7250
$Comp
L Device:D_Schottky D2
U 1 1 607A4E6A
P 1600 4050
F 0 "D2" V 1646 3971 50  0000 R CNN
F 1 "SS14" V 1550 4300 50  0000 R CNN
F 2 "Diode_SMD:D_SOD-123" H 1600 4050 50  0001 C CNN
F 3 "~" H 1600 4050 50  0001 C CNN
	1    1600 4050
	0    -1   -1   0   
$EndComp
$Comp
L Transistor_BJT:BC807 Q1
U 1 1 6078C4F0
P 900 3250
F 0 "Q1" H 1091 3204 50  0000 L CNN
F 1 "BC807" H 1091 3295 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 1100 3175 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC807.pdf" H 900 3250 50  0001 L CNN
	1    900  3250
	-1   0    0    1   
$EndComp
Connection ~ 1100 3250
Wire Wire Line
	800  3600 800  3650
Wire Wire Line
	800  3450 800  3600
Connection ~ 800  3600
$Comp
L Transistor_BJT:BC807 Q2
U 1 1 607883A5
P 1000 3600
F 0 "Q2" H 1191 3554 50  0000 L CNN
F 1 "BC807" H 1191 3645 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 1200 3525 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC807.pdf" H 1000 3600 50  0001 L CNN
	1    1000 3600
	1    0    0    1   
$EndComp
$Comp
L Transistor_BJT:BC807 Q3
U 1 1 6088F9DD
P 4000 1550
F 0 "Q3" H 4191 1504 50  0000 L CNN
F 1 "BC807" H 4191 1595 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4200 1475 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC807.pdf" H 4000 1550 50  0001 L CNN
	1    4000 1550
	-1   0    0    1   
$EndComp
$Comp
L Transistor_BJT:MMBT3904 Q4
U 1 1 607A644D
P 4400 1750
F 0 "Q4" H 4591 1796 50  0000 L CNN
F 1 "MMBT3904" H 3850 1700 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 4600 1675 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N3904.pdf" H 4400 1750 50  0001 L CNN
	1    4400 1750
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4300 1950 4300 2100
Wire Wire Line
	4300 2100 4450 2100
Wire Wire Line
	4600 1750 4700 1750
$Comp
L power:+3.3V #PWR0107
U 1 1 607AD79D
P 4700 1550
F 0 "#PWR0107" H 4700 1400 50  0001 C CNN
F 1 "+3.3V" H 4715 1723 50  0000 C CNN
F 2 "" H 4700 1550 50  0001 C CNN
F 3 "" H 4700 1550 50  0001 C CNN
	1    4700 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 1550 4700 1750
$Comp
L Device:R R3
U 1 1 607A4BEC
P 1350 3100
F 0 "R3" H 1420 3146 50  0000 L CNN
F 1 "1R" H 1420 3055 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 1280 3100 50  0001 C CNN
F 3 "~" H 1350 3100 50  0001 C CNN
	1    1350 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 607A4ED8
P 1600 3100
F 0 "R9" H 1670 3146 50  0000 L CNN
F 1 "1R" H 1670 3055 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 1530 3100 50  0001 C CNN
F 3 "~" H 1600 3100 50  0001 C CNN
	1    1600 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 2950 1350 2950
Wire Wire Line
	1350 3250 1600 3250
Wire Wire Line
	1600 2950 1600 2850
Wire Wire Line
	950  2850 1600 2850
$Comp
L Connector_Generic:Conn_01x06 J4
U 1 1 60803F52
P 2350 1500
F 0 "J4" H 2430 1492 50  0000 L CNN
F 1 "Conn_01x06" H 2430 1401 50  0000 L CNN
F 2 "TwomesGateway:Rj12SMD" H 2350 1500 50  0001 C CNN
F 3 "~" H 2350 1500 50  0001 C CNN
	1    2350 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 1300 1500 1300
Connection ~ 1500 1300
Wire Wire Line
	1400 1400 2150 1400
Wire Wire Line
	1900 1500 2150 1500
Connection ~ 1900 1500
Wire Wire Line
	1900 1800 2150 1800
Connection ~ 1900 1800
Wire Wire Line
	1400 1700 2150 1700
Text Notes 2050 2000 0    50   ~ 0
Additional SMD pads\nfor bottoms side
$Comp
L Device:CP C6
U 1 1 608166A8
P 650 4400
F 0 "C6" H 768 4446 50  0000 L CNN
F 1 "470uF" H 768 4355 50  0000 L CNN
F 2 "Capacitor_SMD:CP_Elec_8x10.5" H 688 4250 50  0001 C CNN
F 3 "~" H 650 4400 50  0001 C CNN
	1    650  4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 4250 650  4250
Wire Wire Line
	650  4550 650  4800
Wire Wire Line
	650  4800 1100 4800
Connection ~ 1100 4800
$Comp
L Device:C C7
U 1 1 608331CC
P 10300 1150
F 0 "C7" H 10415 1196 50  0000 L CNN
F 1 "10uF" H 10400 1100 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 10338 1000 50  0001 C CNN
F 3 "~" H 10300 1150 50  0001 C CNN
	1    10300 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	10300 1000 10050 1000
Connection ~ 9850 1000
Wire Wire Line
	9850 1300 10050 1300
Connection ~ 9850 1300
Connection ~ 10050 1300
Wire Wire Line
	10050 1300 10300 1300
Connection ~ 10050 1000
Wire Wire Line
	10050 1000 9850 1000
$EndSCHEMATC
