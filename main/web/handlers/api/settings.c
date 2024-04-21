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

#include <esp_err.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <cJSON.h>

#include "settings.h"
#include "../../../settings.h"
#include "helper/http.h"
#include "helper/api.h"

static const char *TAG = "WEB/API/SETTINGS";

// List of supported settings
ApiSetting_t settings[] = {
    {"wifi.mode",                        &gSettings.wifi.mode,                        1},
    {"wifi.ssid",                        &gSettings.wifi.ssid,                        0},
    {"wifi.password",                    &gSettings.wifi.password,                    0},
    {"wifi.channel",                     &gSettings.wifi.channel,                     1},
    {"wifi.max_connections",             &gSettings.wifi.max_connections,             1},
    {"gpio.status_led",                  &gSettings.gpio.status_led,                  1},
    {"gpio.audio_in",                    &gSettings.gpio.audio_in,                    1},
    {"gpio.audio_out",                   &gSettings.gpio.audio_out,                   1},
    {"gpio.ptt",                         &gSettings.gpio.ptt,                         1},
    {"audio.out.volume",                 &gSettings.audio.out.volume,                 1},
    {"led.max_brightness",               &gSettings.led.max_brightness,               1},
    {"beacon.mode",                      &gSettings.beacon.mode,                      1},
    {"beacon.text",                      &gSettings.beacon.text,                      0},
    {"beacon.delay_seconds",             &gSettings.beacon.delay_seconds,             1},
    {"beacon.morse_code.baud",           &gSettings.beacon.morse_code.baud,           1},
    {"beacon.morse_code.tone_freq",      &gSettings.beacon.morse_code.tone_freq,      1},
    {"beacon.afsk.baud",                 &gSettings.beacon.afsk.baud,                 1},
    {"beacon.afsk.zero_freq",            &gSettings.beacon.afsk.zero_freq,            1},
    {"beacon.afsk.one_freq",             &gSettings.beacon.afsk.one_freq,             1}
};

// Shows current settings
esp_err_t API_SETTINGS_Index(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request");

    cJSON *root = cJSON_CreateObject();

    // Add all settings to JSON object
    for(u_int8_t i=0; i<(sizeof(settings)/sizeof(settings[0])); i++)
    {
        if(settings[i].isInteger)
        {
            cJSON_AddNumberToObject(root, settings[i].attr,  *(SETTINGS_INTEGER_TYPE *)settings[i].val);
        }
        else
        {
            cJSON_AddStringToObject(root, settings[i].attr,  (const char *)settings[i].val);
        }
    }

    httpd_resp_set_type(req, "application/json");
    const char *json_str = cJSON_Print(root);

    // Send response
    httpd_resp_sendstr(req, json_str);

    // Free memory, it handles all the objects belonging to root
    cJSON_Delete(root);

    return ESP_OK;
}

/* Sets device settings, saves them and restarts the device

to test:
curl -d '{
              "wifi.mode":1,
              "wifi.ssid":"SAMSUNG-S3",
              "wifi.password":"testtesttest123",
              "wifi.channel":3,
              "wifi.max_connections":3
              }' http://192.168.4.1.api/settings */
esp_err_t API_SETTINGS_Create(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((file_server_data *)(req->user_ctx))->scratch;
    int received = 0;

    // Read request content
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            httpd_json_resp_send(req, HTTPD_500, "Failed to process request");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    // Parse JSON
    cJSON *root = cJSON_ParseWithLength(buf, total_len);

    // Handle invalid JSON
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Invalid JSON received");
        httpd_json_resp_send(req, HTTPD_500, "Invalid JSON received");
        // Free memory
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Get all settings from the JSON object
    for(u_int8_t i=0; i<(sizeof(settings)/sizeof(settings[0])); i++)
    {
        cJSON *attr = cJSON_GetObjectItem(root, settings[i].attr);
        // if attr is null we go to next item
        if (attr == NULL) continue;
        // set the setting based on the data type
        if(settings[i].isInteger)
        {
            *(SETTINGS_INTEGER_TYPE *)settings[i].val = cJSON_GetNumberValue(attr);
        }
        else
        {
            strcpy((char *)settings[i].val, cJSON_GetStringValue(attr));
        }
    }
    // Free memory, it handles both root and attr
    cJSON_Delete(root);
    
    httpd_json_resp_send(req, HTTPD_200, "Settings saved.");

    SETTINGS_Save();

    return ESP_OK;
}