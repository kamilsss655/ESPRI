/* Copyright 2024 kamilsss655
 * https://github.com/kamilsss655
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "hardware/uart.h"
#include "hardware/wifi.h"
#include "hardware/spiffs.h"

// Initialize the board
void BOARD_init(void)
{
    // Initialize SPIFFS
    const char *base_path = "/storage";
    SPIFFS_init(base_path);
    // Initialize UART
    UART_init();
    // Initialize WIFI
    WIFI_init();
}