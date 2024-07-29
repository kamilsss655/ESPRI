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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <esp_err.h>

#include "helper/api.h"

#include "board.h"

#define CONFIG_FILE_PATH "/config.bin"
// Determines location of where the config file is stored ie. /storage/config.bin
#define CONFIG_LOCATION FLASH_BASE_PATH CONFIG_FILE_PATH

// BOOL type
typedef enum
{
    SETTINGS_FALSE,
    SETTINGS_TRUE
} SETTINGS_Bool_t;

// WIFI mode
typedef enum
{
    SETTINGS_WIFI_MODE_OFF,
    SETTINGS_WIFI_MODE_AP,
    SETTINGS_WIFI_MODE_STA
} SETTINGS_WifiMode_t;

// SETTINGS_WIFI_channel
typedef enum
{
    SETTINGS_WIFI_CHANNEL_1 = 1,
    SETTINGS_WIFI_CHANNEL_2,
    SETTINGS_WIFI_CHANNEL_3,
    SETTINGS_WIFI_CHANNEL_4,
    SETTINGS_WIFI_CHANNEL_5,
    SETTINGS_WIFI_CHANNEL_6,
    SETTINGS_WIFI_CHANNEL_7,
    SETTINGS_WIFI_CHANNEL_8,
    SETTINGS_WIFI_CHANNEL_9,
    SETTINGS_WIFI_CHANNEL_10,
    SETTINGS_WIFI_CHANNEL_11,
    SETTINGS_WIFI_CHANNEL_12,
    SETTINGS_WIFI_CHANNEL_13,
} SETTINGS_WifiChannel_t;

// SETTINGS_WIFI_max stations connected
typedef enum
{
    SETTINGS_WIFI_MAX_CONN_1  = 1,
    SETTINGS_WIFI_MAX_CONN_5  = 5,
    SETTINGS_WIFI_MAX_CONN_10 = 10,
    SETTINGS_WIFI_MAX_CONN_30 = 30
} SETTINGS_WifiMaxConn_t;

// Wifi settings
typedef struct
{
    SETTINGS_WifiMode_t     mode;
    char                    ssid[32];
    char                    password[64];
    SETTINGS_WifiChannel_t  channel;
    SETTINGS_WifiMaxConn_t  max_connections;
} SETTINGS_WifiConfig_t;

// GPIO settings
typedef struct
{
    API_INTEGER_TYPE status_led;
    API_INTEGER_TYPE audio_out;
    API_INTEGER_TYPE audio_in;
    API_INTEGER_TYPE ptt;
} SETTINGS_GpioConfig_t;

// Beacon mode
typedef enum {
    SETTINGS_BEACON_MODE_OFF,
    SETTINGS_BEACON_MODE_MORSE_CODE,
    SETTINGS_BEACON_MODE_AFSK,
    SETTINGS_BEACON_MODE_WAV
} SETTINGS_BeaconMode_t;

// Morse code beacon settings
typedef struct
{
    API_INTEGER_TYPE baud;
    API_INTEGER_TYPE tone_freq;
    char             text[64];
} SETTINGS_MorseCodeBeaconConfig_t;

// AFSK beacon settings
typedef struct
{
    API_INTEGER_TYPE baud;
    API_INTEGER_TYPE zero_freq;
    API_INTEGER_TYPE one_freq;
} SETTINGS_AfskBeaconConfig_t;

// WAV beacon settings
typedef struct
{
    char filepath[64]; // path to .wav file
} SETTINGS_WavBeaconConfig_t;

// Audio out settings
typedef struct
{
    API_INTEGER_TYPE volume; // 0-100 - determines audio output volume
} SETTINGS_AudioOutConfig_t;

// Audio in settings
typedef struct
{
    API_INTEGER_TYPE squelch; // 0-100 - determines squelch sensitivity
} SETTINGS_AudioInConfig_t;

// Audio settings
typedef struct
{
    SETTINGS_AudioOutConfig_t out;
    SETTINGS_AudioInConfig_t  in;
} SETTINGS_AudioConfig_t;

// LED settings
typedef struct
{
    API_INTEGER_TYPE max_brightness;
} SETTINGS_LedConfig_t;

// Beacon settings
typedef struct
{
    SETTINGS_BeaconMode_t            mode;
    char                             text[64];
    API_INTEGER_TYPE            delay_seconds;
    SETTINGS_MorseCodeBeaconConfig_t morse_code;
    SETTINGS_AfskBeaconConfig_t      afsk;
    SETTINGS_WavBeaconConfig_t       wav;
} SETTINGS_BeaconConfig_t;

// Calibration subtype
typedef struct
{
    API_INTEGER_TYPE value;    // value when there is no audio in signal
    SETTINGS_Bool_t       is_valid; // set it to false to trigger calibration
} SETTINGS_CalibrationSubtype_t;

// Calibration settings
typedef struct
{
    SETTINGS_CalibrationSubtype_t adc;
    // SETTINGS_CalibrationSubtype_t touch;
} SETTINGS_Calibration_t;

// Global settings
typedef struct
{
    char                             padding[4];
    SETTINGS_WifiConfig_t            wifi;
    SETTINGS_GpioConfig_t            gpio;
    SETTINGS_AudioConfig_t           audio;
    SETTINGS_LedConfig_t             led;
    SETTINGS_BeaconConfig_t          beacon;
    SETTINGS_Calibration_t           calibration;
} SETTINGS_Config_t;

extern SETTINGS_Config_t gSettings;

// Initialize the board
esp_err_t SETTINGS_Init(void);
esp_err_t SETTINGS_Load(void);
esp_err_t SETTINGS_Save(void);
esp_err_t SETTINGS_FactoryReset(bool reboot);

#endif