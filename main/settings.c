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

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>
#include <esp_log.h>
#include <esp_spiffs.h>

#include "settings.h"
#include "system.h"
#include "app/morse.h"

static const char *TAG = "SETTINGS";

SETTINGS_Config_t gSettings;

SemaphoreHandle_t settingsSemaphore;

// Initialize settings
esp_err_t SETTINGS_Init(void)
{
    settingsSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(settingsSemaphore);

    ESP_ERROR_CHECK(SETTINGS_Load());

    return ESP_OK;
}

// Load settings from local filesystem
esp_err_t SETTINGS_Load(void)
{
    if (xSemaphoreTake(settingsSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
    {
        ESP_LOGE(TAG, "Could not load settings due to lock");
        return ESP_FAIL;
    }

    FILE *fd = NULL;

    fd = fopen(CONFIG_LOCATION, "rb");
    if (!fd)
    {
        ESP_LOGE(TAG, "No config file found. First boot?");
        xSemaphoreGive(settingsSemaphore);
        SYSTEM_FirstBoot();
    }
    else
    {
        fread(&gSettings, 1, sizeof(gSettings), fd);
        fclose(fd);
        xSemaphoreGive(settingsSemaphore);
    }

    return ESP_OK;
}

// Save settings into local filesystem
esp_err_t SETTINGS_Save(void)
{
    if (xSemaphoreTake(settingsSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
    {
        ESP_LOGE(TAG, "Could not save settings due to lock");
        return ESP_FAIL;
    }

    FILE *fd = NULL;

    fd = fopen(CONFIG_LOCATION, "wb");
    fwrite(&gSettings, 1, sizeof(gSettings), fd);
    fclose(fd);

    xSemaphoreGive(settingsSemaphore);

    return ESP_OK;
}

/// @brief Performs factory reset
// Allows to overwrite settings with private developer settings
// To use run: idp.py menuconfig -> Project configuration
// Settings set with menuconfig are not tracked with version control
// Useful for connecting to private WIFI network
/// @param reboot determines whether device should reboot after the reset
/// @return
esp_err_t SETTINGS_FactoryReset(bool reboot)
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
    gSettings.gpio.audio_in   = CONFIG_AUDIO_IN_GPIO;
    gSettings.gpio.status_led = CONFIG_STATUS_LED_GPIO;
    gSettings.gpio.ptt        = CONFIG_PTT_GPIO;
    // Audio
    gSettings.audio.out.volume = CONFIG_AUDIO_OUT_VOLUME;
    gSettings.audio.in.squelch = CONFIG_AUDIO_IN_SQUELCH;
    // LED
    gSettings.led.max_brightness = CONFIG_STATUS_LED_GPIO_MAX_BRIGHTNESS;
    // Beacon
    gSettings.beacon.mode = CONFIG_BEACON_MODE;
    gSettings.beacon.delay_seconds = CONFIG_BEACON_DELAY_SECONDS;
    strcpy(gSettings.beacon.text, CONFIG_BEACON_TEXT);
    // Morse code beacon
    gSettings.beacon.morse_code.tone_freq = CONFIG_MORSE_CODE_BEACON_TONE_FREQ;
    gSettings.beacon.morse_code.baud = CONFIG_MORSE_CODE_BEACON_BAUD;
    // AFSK beacon
    gSettings.beacon.afsk.baud = CONFIG_AFSK_BEACON_BAUD;
    gSettings.beacon.afsk.zero_freq = CONFIG_AFSK_ZERO_FREQ;
    gSettings.beacon.afsk.one_freq = CONFIG_AFSK_ONE_FREQ;
    // WAV beacon
    strcpy(gSettings.beacon.wav.filepath, CONFIG_WAV_BEACON_FILEPATH);
    // Calibration
    gSettings.calibration.adc.value = 0;
    gSettings.calibration.adc.is_valid = SETTINGS_FALSE;

    SETTINGS_Save();

    if (reboot)
    {
        SYSTEM_Reboot();
    }

    return ESP_OK;
}