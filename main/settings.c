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
#include <esp_system.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include "settings.h"

static const char *TAG = "SETTINGS";

SETTINGS_Config_t gSettings;

// Initialize the board
esp_err_t SETTINGS_Load(void)
{

    FILE *fd = NULL;

    fd = fopen("/storage/config", "rb");
    if (!fd)
    {
        ESP_LOGI(TAG, "No config file found.");
        SETTINGS_FactoryReset();
    }
    fread(&gSettings, 1, sizeof(gSettings),fd);
    fclose(fd);

    return ESP_OK;
}

esp_err_t SETTINGS_Save(void)
{
    FILE *fd = NULL;

    fd = fopen("/storage/config", "wb");
    fwrite(&gSettings, 1, sizeof(gSettings),fd);
    fclose(fd);

    return ESP_OK;
}

esp_err_t SETTINGS_FactoryReset(void)
{
    ESP_LOGW(TAG, "Performing factory reset.");

    gSettings.wifi.mode = SETTINGS_WIFI_MODE_AP;
    strcpy(gSettings.wifi.ssid, "NOKIA-4V5O1F0");
    strcpy(gSettings.wifi.password, "mypassword");
    gSettings.wifi.channel = SETTINGS_WIFI_CHANNEL_6;
    gSettings.wifi.max_connections = SETTINGS_WIFI_MAX_CONN_5;

    #ifdef CONFIG_DEVELOPER_MODE
        SETTINGS_LoadDeveloperMode();
    #endif

    SETTINGS_Save();
    
    esp_restart();
    
    return ESP_OK;
}

#ifdef CONFIG_DEVELOPER_MODE
// Allows to overwrite settings with private developer settings
// To use run: idp.py menuconfig -> Developer configuration
// Settings set with menuconfig are not tracked with version control
// Useful for connecting to private WIFI network
void SETTINGS_LoadDeveloperMode(void)
{
    #ifdef CONFIG_DEVELOPER_WIFI_AP_MODE_ENABLED
        gSettings.wifi.mode = SETTINGS_WIFI_MODE_AP;
    #else
        gSettings.wifi.mode = SETTINGS_WIFI_MODE_STA;
    #endif
    strcpy(gSettings.wifi.ssid, CONFIG_DEVELOPER_WIFI_SSID);
    strcpy(gSettings.wifi.password, CONFIG_DEVELOPER_WIFI_PASSWORD);
    gSettings.wifi.channel = CONFIG_DEVELOPER_WIFI_CHANNEL;
    gSettings.wifi.max_connections = CONFIG_DEVELOPER_MAX_STA_CONN;
}
#endif