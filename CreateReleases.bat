@echo off

echo MAKING P1_ONLY release:
pio run -e P1_ONLY

mkdir .\binaries\P1Gateway_only

copy ".\.pio\build\P1_ONLY\bootloader.bin" ".\binaries\P1Gateway_only"
copy ".\.pio\build\P1_ONLY\partitions.bin" ".\binaries\P1Gateway_only"
copy ".\.pio\build\P1_ONLY\firmware.bin" ".\binaries\P1Gateway_only"
copy ".\.pio\build\P1_ONLY\ota_data_initial.bin" ".\binaries\P1Gateway_only"

echo MAKING P1_TinTsTr release:
pio run -e P1_TinTsTr

mkdir .\binaries\P1Gateway_TinTsTr

copy ".\.pio\build\P1_TinTsTr\bootloader.bin" ".\binaries\P1Gateway_TinTsTr"
copy ".\.pio\build\P1_TinTsTr\partitions.bin" ".\binaries\P1Gateway_TinTsTr"
copy ".\.pio\build\P1_TinTsTr\firmware.bin" ".\binaries\P1Gateway_TinTsTr"
copy ".\.pio\build\P1_TinTsTr\ota_data_initial.bin" ".\binaries\P1Gateway_TinTsTr"

echo MAKING P1_TinTsTrCO2 release:
pio run -e P1_TinTsTrCO2

mkdir .\binaries\P1Gateway_TinTsTrCO2

copy ".\.pio\build\P1_TinTsTrCO2\bootloader.bin" ".\binaries\P1Gateway_TinTsTrCO2"
copy ".\.pio\build\P1_TinTsTrCO2\partitions.bin" ".\binaries\P1Gateway_TinTsTrCO2"
copy ".\.pio\build\P1_TinTsTrCO2\firmware.bin" ".\binaries\P1Gateway_TinTsTrCO2"
copy ".\.pio\build\P1_TinTsTrCO2\ota_data_initial.bin" ".\binaries\P1Gateway_TinTsTrCO2"

echo MAKING P1_CO2 release:
pio run -e P1_CO2

mkdir .\binaries\P1Gateway_CO2

copy ".\.pio\build\P1_CO2\bootloader.bin" ".\binaries\P1Gateway_CO2"
copy ".\.pio\build\P1_CO2\partitions.bin" ".\binaries\P1Gateway_CO2"
copy ".\.pio\build\P1_CO2\firmware.bin" ".\binaries\P1Gateway_CO2"
copy ".\.pio\build\P1_CO2\ota_data_initial.bin" ".\binaries\P1Gateway_CO2"

echo DONE
