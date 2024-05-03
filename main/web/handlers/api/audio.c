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

    esp_err_t ret = process_api_attributes(req, TAG, record_attributes, (sizeof(record_attributes) / sizeof(record_attributes[0])));

    // If processing attributes resulted in error we return early
    if(ret != ESP_OK)
    {
        return ret;
    }

    if (audioRecordTaskHandle == NULL)
    {
        xTaskCreate(AUDIO_Record, audioRecordTaskName, 4096, &record_param, RTOS_PRIORITY_MEDIUM, NULL);
        httpd_json_resp_send(req, HTTPD_200, "OK. Recording will start once the squelch opens.");
    }
    else
    {
        httpd_json_resp_send(req, HTTPD_500, "Recording task is already running.");
    }

    return ESP_OK;
}