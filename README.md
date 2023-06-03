# Moisture Sensor Firmware

-   Main branch @ commit 80a22dc22965714d2e0cb4b83949878685d2aa73 https://github.com/espressif/esp-matter

## Clone

```bash
git clone --recursive https://github.com/tobyloki/MoistureSensorFirmware.git
```

```bash
git submodule update --init --recursive
```

## Setup

```bash
./esp-matter/install.sh
chmod +x ./esp-matter/export.sh
```

```bash
. ~/matter/esp-idf/export.sh
. ./esp-matter/export.sh
```

## Build

```bash
cd firmware
idf.py build
```

## Flash

```bash
idf.py flash monitor
```

## Using pre-built-image

Read instructions in [pre-built-image](./pre-built-image/README.md)

## Commissioning Note

-   Make sure to disable VPN before commissioning
-   Ensure WiFi network supports 2.4GHz
