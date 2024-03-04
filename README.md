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

## Building
This project uses Docker to build binaries.

1. To compile:
```
make docker
```

## References

- https://github.com/skuep/AIOC
- http://jtxp.org/tech/hektor_en.htm
- https://en.wikipedia.org/wiki/Delta-sigma_modulation
- https://en.wikipedia.org/wiki/Pulse-density_modulation
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/sdm.html
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32h2/api-reference/peripherals/ana_cmpr.html

## License

Modified work Copyright 2024 kamilsss655
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
