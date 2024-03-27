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

#include "hardware/audio.h"
#include "hardware/http_server.h"
#include "hardware/uart.h"
#include "hardware/wifi.h"
#include "hardware/button.h"
#include "hardware/spiffs.h"
#include "hardware/ptt.h"
#include "settings.h"
#include "board.h"

// Initialize the board
void BOARD_Init(void)
{
    // Initialize SPIFFS
    ESP_ERROR_CHECK(SPIFFS_Init(STORAGE_BASE_PATH));
    // Load SETTINGS
    ESP_ERROR_CHECK(SETTINGS_Load());

    // Initialize UART
    UART_Init();
    // Initialize WIFI
    WIFI_Init();
    // Initialize HTTP Server
    ESP_ERROR_CHECK(HTTP_SERVER_Init(STORAGE_BASE_PATH));
    // Initialize button

    BUTTON_Init();
    // Initialize PTT
    ESP_ERROR_CHECK(PTT_Init());
    AUDIO_Init();
}