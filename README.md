# Moisture Sensor Firmware

- Main branch @ commit 80a22dc22965714d2e0cb4b83949878685d2aa73 https://github.com/espressif/esp-matter

## Prerequisites

- Follow pre-reqs for esp-idf and matter setup at https://docs.espressif.com/projects/esp-matter/en/main/esp32/developing.html

## Clone

```bash
git clone --recursive https://github.com/tobyloki/MoistureSensorFirmware.git
```

```bash
git submodule update --init --recursive
```

## Setup

Note: Execute all these commands from within the root directory

```bash
./esp-matter/install.sh
chmod +x ./esp-matter/export.sh
```

```bash
. ~/matter/esp-idf/export.sh
. ./esp-matter/export.sh
```

- Copy modified files
  - Added missing humidity cluster and device type to esp-matter
  - Copy arduino libraries
  ```bash
  chmod +x ./initialFiles/copyFiles.sh
  ./initialFiles/copyFiles.sh
  ```

## Build

```bash
cd firmware
idf.py build
```

## Flash

```bash
idf.py erase-flash flash monitor
idf.py flash monitor
```

## Commissioning Notes

- Make sure to disable VPN before commissioning
- Ensure WiFi network supports 2.4GHz

## Arduino Notes

- Follow instructions at https://github.com/espressif/esp-matter/issues/116#issuecomment-1356673244 which explains how to use arduino-esp32 library with esp-matter
  - Includes basic instructions from https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/esp-idf_component.html with extra info on how to fix errors when using with esp-matter
  - Currently solved by running the copyFiles.sh command from #Setup section
- To add more Arduino libraries

  1. Clone library as submodule

  ```bash
  cd ./initialFiles/arduino/
  git submodule add <repo>
  git submodule update --init --recursive
  ```

  2. Add reference to library include and src files to `./initialFiles/arduino/CMakeLists.txt`

  ```cmake
  set(LIBRARY_SRCS
    ...
    libraries/arduino-sht/SHTSensor.cpp
  )

  ...

  set(includedirs
    ...
    libraries/arduino-sht
  )
  ```

  3. Update `./initialFiles/copyFiles.sh` to copy new arduino library to arduino libraries directory

  4. Run `./initialFiles/copyFiles.sh`
