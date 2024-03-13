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

#include <string.h>
#include "settings.h"

SETTINGS_Config_t gSettings;

// Initialize the board
esp_err_t SETTINGS_Load(void)
{
    gSettings.wifi.mode = SETTINGS_WIFI_MODE_AP;
    strcpy(gSettings.wifi.ssid, "NOKIA-4V5O1F0");
    strcpy(gSettings.wifi.password, "mypassword");
    gSettings.wifi.channel = SETTINGS_WIFI_CHANNEL_6;
    gSettings.wifi.max_connections = SETTINGS_WIFI_MAX_CONN_5;
    return ESP_OK;
}

esp_err_t SETTINGS_Save(void)
{
    return ESP_OK;
}