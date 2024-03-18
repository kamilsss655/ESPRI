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

#include "../../../settings.h"
static const char *TAG = "WEB/API/SETTINGS";

esp_err_t API_SETTINGS_Index(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received request");

    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "wifi.mode", gSettings.wifi.mode);
    cJSON_AddStringToObject(root, "wifi.ssid", gSettings.wifi.ssid);
    cJSON_AddStringToObject(root, "wifi.password", gSettings.wifi.password);
    cJSON_AddNumberToObject(root, "wifi.channel", gSettings.wifi.channel);
    cJSON_AddNumberToObject(root, "wifi.max_connections", gSettings.wifi.max_connections);

    httpd_resp_set_type(req, "application/json");
    const char *json_str = cJSON_Print(root);
    httpd_resp_sendstr(req, json_str);

    // Free memory, it handles both root and attr
    cJSON_Delete(root);

    return ESP_OK;
}