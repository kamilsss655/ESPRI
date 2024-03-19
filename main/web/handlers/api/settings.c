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

static const char *TAG = "WEB/API/SETTINGS";

ApiSetting_t settings[6] = {
    {"wifi.mode",            &gSettings.wifi.mode,            1},
    {"wifi.ssid",            &gSettings.wifi.ssid,            0},
    {"wifi.password",        &gSettings.wifi.password,        0},
    {"wifi.channel",         &gSettings.wifi.channel,         1},
    {"wifi.max_connections", &gSettings.wifi.max_connections, 1}};

esp_err_t API_SETTINGS_Index(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request");

    cJSON *root = cJSON_CreateObject();

    // Add all settings to JSON object
    for(u_int8_t i=0; i<(sizeof(settings)/sizeof(settings[0])); i++)
    {
        if(settings[i].isInteger)
        {
            cJSON_AddNumberToObject(root, settings[i].attr,  *(u_int32_t *)settings[i].val);
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