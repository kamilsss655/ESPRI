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

// Load settings from local filesystem
esp_err_t SETTINGS_Load(void)
{

    FILE *fd = NULL;

    fd = fopen(CONFIG_LOCATION, "rb");
    if (!fd)
    {
        ESP_LOGE(TAG, "No config file found.");
        SETTINGS_FactoryReset();
    }
    fread(&gSettings, 1, sizeof(gSettings),fd);
    fclose(fd);

    return ESP_OK;
}

// Save settings into local filesystem
esp_err_t SETTINGS_Save(void)
{
    FILE *fd = NULL;

    fd = fopen(CONFIG_LOCATION, "wb");
    fwrite(&gSettings, 1, sizeof(gSettings),fd);
    fclose(fd);

    return ESP_OK;
}

// Performs factory reset
// Allows to overwrite settings with private developer settings
// To use run: idp.py menuconfig -> Project configuration
// Settings set with menuconfig are not tracked with version control
// Useful for connecting to private WIFI network
esp_err_t SETTINGS_FactoryReset(void)
{
    ESP_LOGW(TAG, "Performing factory reset.");
    // WiFi
    #ifdef CONFIG_WIFI_AP_MODE_ENABLED
        gSettings.wifi.mode = SETTINGS_WIFI_MODE_AP;
        gSettings.wifi.channel = CONFIG_WIFI_CHANNEL;
        gSettings.wifi.max_connections = CONFIG_MAX_STA_CONN;
    #else
        gSettings.wifi.mode = SETTINGS_WIFI_MODE_STA;
    #endif
    strcpy(gSettings.wifi.ssid, CONFIG_WIFI_SSID);
    strcpy(gSettings.wifi.password, CONFIG_WIFI_PASSWORD);
    // GPIO
    gSettings.gpio.audio_out  = CONFIG_AUDIO_OUT_GPIO;
    gSettings.gpio.status_led = CONFIG_STATUS_LED_GPIO;
    gSettings.gpio.ptt        = CONFIG_PTT_GPIO;
    // Morse code beacon
    gSettings.morse_code_beacon.enabled = CONFIG_MORSE_CODE_BEACON_ENABLED;
    gSettings.morse_code_beacon.period_seconds = CONFIG_MORSE_CODE_BEACON_PERIOD_SECONDS;
    strcpy(gSettings.morse_code_beacon.text, CONFIG_MORSE_CODE_BEACON_TEXT);


    SETTINGS_Save();
    
    esp_restart();
    
    return ESP_OK;
}