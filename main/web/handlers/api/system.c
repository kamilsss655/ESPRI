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

#include <esp_system.h>
#include <esp_sleep.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>

#include "system.h"
#include "../../main/system.h"
#include "../../main/settings.h"
#include "helper/api.h"
#include "helper/rtos.h"

static const char *TAG = "WEB/API/SYSTEM";

// List of supported settings
SystemInfo_t systemInfo[] = {
    {"heap.total",    &gSystemInfo.heap.total,    1},
    {"heap.free",     &gSystemInfo.heap.free,     1},
    {"heap.min_free", &gSystemInfo.heap.min_free, 1},
    {"storage.free",  &gSystemInfo.storage.free,  1},
    {"storage.total", &gSystemInfo.storage.total, 1},
    {"uptime",        &gSystemInfo.uptime,        1},
    {"version",       &gSystemInfo.version,       0}
};

// System info
esp_err_t API_SYSTEM_Info(httpd_req_t *req)
{

    cJSON *root = cJSON_CreateObject();

    // Add all system info to JSON object
    for (u_int8_t i = 0; i < (sizeof(systemInfo) / sizeof(systemInfo[0])); i++)
    {
        if (systemInfo[i].isInteger)
        {
            cJSON_AddNumberToObject(root, systemInfo[i].attr, *(SYSTEM_INTEGER_TYPE *)systemInfo[i].val);
        }
        else
        {
            cJSON_AddStringToObject(root, systemInfo[i].attr, (const char *)systemInfo[i].val);
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

// Reboot the device
esp_err_t API_SYSTEM_Reboot(httpd_req_t *req)
{
    ESP_LOGW(TAG, "Performing system reboot..");

    httpd_json_resp_send(req, HTTPD_200, "OK. Restarting device.");

    // Delay to respond to client gracefully
    vTaskDelay(10);

    esp_restart();

    return ESP_OK;
}

// Deep sleep
esp_err_t API_SYSTEM_DeepSleep(httpd_req_t *req)
{
    ESP_LOGW(TAG, "Going into deep sleep..");

    httpd_json_resp_send(req, HTTPD_200, "OK. Going into deep sleep.");

    // Delay to respond to client gracefully
    vTaskDelay(10);

    esp_deep_sleep_start();

    return ESP_OK;
}

// Perform factory reset
esp_err_t API_SYSTEM_FactoryReset(httpd_req_t *req)
{
    ESP_LOGW(TAG, "Performing factory reset..");

    httpd_json_resp_send(req, HTTPD_200, "OK. Performing factory reset.");

    // Delay to respond to client gracefully
    vTaskDelay(10);

    SETTINGS_FactoryReset();

    return ESP_OK;
}