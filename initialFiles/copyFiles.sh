#!/bin/sh

# matter
cp -r ./initialFiles/matter/components/* ./esp-matter/components/esp_matter/
cp ./initialFiles/matter/IPAddress.h ./esp-matter/connectedhomeip/connectedhomeip/src/inet/

# arduino
cp -r ./initialFiles/arduino/arduino-sht/ ./components/arduino-esp32/libraries/
cp ./initialFiles/arduino/CMakeLists.txt ./components/arduino-esp32/