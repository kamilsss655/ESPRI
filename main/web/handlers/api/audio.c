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
#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>

#include "hardware/audio.h"
#include "helper/rtos.h"
#include "helper/api.h"
#include "helper/http.h"

static const char *TAG = "WEB/API/AUDIO";
static const char *audioRecordTaskName = "AUDIO_Record";

// Default values
AUDIO_RecordParam_t record_param = {
    .filepath = "sample.wav",
    .max_duration_ms = 2000
};

// List of record attributes
ApiAttr_t record_attributes[] = {
    {"filepath",                         &record_param.filepath,                      0},
    {"max_duration_ms",                  &record_param.max_duration_ms,               1}
};

// Schedule audio record task
esp_err_t API_AUDIO_Record(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received record request");

    // Check if there is other instance of the task running
    TaskHandle_t audioRecordTaskHandle = xTaskGetHandle(audioRecordTaskName);

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
    for(u_int8_t i=0; i<(sizeof(record_attributes)/sizeof(record_attributes[0])); i++)
    {
        cJSON *attr = cJSON_GetObjectItem(root, record_attributes[i].attr);
        // if attr is null we go to next item
        if (attr == NULL) continue;
        // set the setting based on the data type
        if(record_attributes[i].isInteger)
        {
            *(API_INTEGER_TYPE *)record_attributes[i].val = cJSON_GetNumberValue(attr);
        }
        else
        {
            strcpy((char *)record_attributes[i].val, cJSON_GetStringValue(attr));
        }
    }
    // Free memory, it handles both root and attr
    cJSON_Delete(root);


    if (audioRecordTaskHandle == NULL)
    {
        xTaskCreate(AUDIO_Record, audioRecordTaskName, 4096, &record_param, RTOS_PRIORITY_MEDIUM, NULL);
        httpd_json_resp_send(req, HTTPD_200, "OK. Recording scheduled.");
    }
    else
    {
        httpd_json_resp_send(req, HTTPD_500, "Recording is already in progress.");
    }

    return ESP_OK;
}