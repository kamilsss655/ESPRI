# ESPRI
ESPRI (ESP Radio Interface) project providing extended functionalities to ham radios.

![high-level-diagram-3](https://github.com/kamilsss655/ESPRI/assets/8842065/868cc9b6-6143-4308-a15c-6a4fdc008033)

The main goal of this project is to extend functionalities of ham radios with addition of a small PCB board that contains ESP32 microcontroller.

As such following features would be enabled:
- time domain scrambling (based on the [HEKTOR 128](http://jtxp.org/tech/hektor_en.htm) project)
- custom digital modes (modulated and demodulated by the ESP chip)
  - more advanced messaging
    - could be served as webpage by the esp chip itself
    - or have a mobile client as Meshtastic 
- wireless UART connection to the radio

The physicial inteface layer between the ESPRI and the host radio will be through the Kenwood connector.

The audio input and output (for the means of time domain scrambling) would be through the Kenwood connector.

## Dependencies

This firmware uses:
* Espressif IoT Development Framework (ESP-IDF) `v5.2.1`

## Development

### With Visual Studio Code and ESP-IDF extension

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

#### To fix Visual Studio Code missing defintions to ESP-IDF functions:

This method will perform build within the Docker container.

PROS:
* Clean install.

CONS:
* IntelliSense in Visual Studio Code will not work (due to weird ESP-IDF structure and dynamic file inclusion)

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
