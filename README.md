# ESPRI
ESPRI (ESP Radio Interface) project providing extended functionalities to ham radios.

![high-level-diagram-4](https://github.com/kamilsss655/ESPRI/assets/8842065/813055e7-6729-47e8-8912-1e9155d7ca6d)

The main goal of this project is to extend functionalities of ham radios with addition of a small PCB board that contains ESP32 microcontroller.

As such following features would be enabled:
- time domain scrambling (based on the [HEKTOR 128](http://jtxp.org/tech/hektor_en.htm) project)
- custom digital modes (modulated and demodulated by the ESP chip)
  - more advanced messaging
    - could be served as webpage by the esp chip itself
    - or have a mobile client as Meshtastic 
- wireless UART connection to the radio
- turning any radio into IoT device via JSON API served from the ESP

The physicial inteface layer between the ESPRI and the host radio will be through the Kenwood connector.

The audio input and output (for the means of time domain scrambling) would be through the Kenwood connector.

## Web panel

Here is a screenshot of a web control panel being served from the ESP:

![espri-new-ui](https://github.com/kamilsss655/ESPRI/assets/8842065/a17e3bd1-4b6b-48cd-8fb4-0371cf43ec07)

Morse code beacon config:

![morse](https://github.com/kamilsss655/ESPRI/assets/8842065/c34a8f98-b447-408b-ab64-517cab04be98)


ESP Log:
```
I (8732) WEB/API/EVENT_HANDLER: Event id:3 received
I (8732) WEB/API/EVENT_HANDLER: Sent: SMS: API event: 3 received.
```

## Dependencies
Hardware:
* ESP32 or ESP32 S3

Some of the recommended boards:

ESP32 Lolin Lite (confirmed to work):

![esp32-lolin-lite](https://github.com/kamilsss655/ESPRI/assets/8842065/2e74e723-4347-4efb-b17b-20b4e930523f)

ESP32 S3 Zero (confirmed to work):

![esp32-s3-zero](https://github.com/kamilsss655/ESPRI/assets/8842065/da0c5b13-dae1-43ec-b053-b3f1df78d63b)



This firmware uses:
* Espressif IoT Development Framework (ESP-IDF) `v5.2.1`

## Getting started

> [!TIP]
> You can flash this firmware with [esptool.py](https://docs.espressif.com/projects/esptool/en/latest/esp32/)
> or with [ESP-Launchpad](https://espressif.github.io/esp-launchpad/) (using web browser)

To flash with `esptool.py`:

`esptool.py write_flash 0x0 espri.bin`

Once flashed:
- Connect to WIFI access point `NOKIA-3K9N4H1` with password `mypassword`
- Go to `http://192.168.4.1`

## Development

### With Visual Studio Code and ESP-IDF extension (recommended)

This method will install ESP-IDF toolchain locally with the help of the `ESP-IDF` Visual Studio Code extension.

PROS:
* IntelliSense in Visual Studio Code will work and errors are linted live during development
* Flashing, building, monitoring can be done directly from the Visual Studio Code

CONS:
* It will install toolchain locally

STEPS:
1. Install Visual Studio Code.
2. Install ESP-IDF Visual Studio extension.
3. Perform `EXPRESS` configuration and select correct ESP-IDF version

### With Docker

This method will perform build within the Docker container.

PROS:
* Clean install.

CONS:
* IntelliSense in Visual Studio Code will not work (due to weird ESP-IDF structure and dynamic file inclusion)

STEPS:
1. Download ESP-IDF library:
```
git clone --recursive https://github.com/espressif/esp-idf.git
```

2. Remove `examples` folder:
```
rm -rf esp-idf/examples
```

3. Remove `.git` folder:
```
rm -rf esp-idf/.git
```

4. Update VS Code include PATH (file `c_cpp_properties.json`):

```
 "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/esp-idf/components/**"
            ],
```

5. Install esptool:
```
pip3 install --user esptool
```

6. Build:
```
make docker
```

7. Flash:
```
make flash
```

## How to contribute

Thank you for your interest in contributing to this project! Here are some of the many ways in which you can help:

* Report bugs
* Suggest improvements
* Audit the source code
* Submit PRs to fix bugs or implement features
* Test new releases and updates
* Record videos and post on other platforms
* Spread the word about this project
* Write and edit the documentation

## References

- https://github.com/skuep/AIOC
- http://jtxp.org/tech/hektor_en.htm
- https://en.wikipedia.org/wiki/Delta-sigma_modulation
- https://en.wikipedia.org/wiki/Pulse-density_modulation
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sdm.html
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32h2/api-reference/peripherals/ana_cmpr.html

## License

Copyright 2024 kamilsss655
https://github.com/kamilsss655

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
