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
#include <app/transmit.h>

static const char *TAG = "WEB/API/AUDIO";

static const char *audioRecordTaskName = "AUDIO_Record";
static const char *audioTransmitWAVTaskName = "TRANSMIT_Wav";

// Default values
AUDIO_RecordParam_t record_param = {
    .filepath = AUDIO_DEFAULT_WAV_SAMPLE_FILEPATH,
    .duration_sec = 10};

TRANSMIT_WavParam_t transmit_wav_param = {
    .filepath = AUDIO_DEFAULT_WAV_SAMPLE_FILEPATH};

// List of audio record attributes
ApiAttr_t record_attributes[] = {
    {"filepath", &record_param.filepath, 0},
    {"duration_sec", &record_param.duration_sec, 1}};

// List of audio transmit WAV attributes
ApiAttr_t transmit_wav_attributes[] = {
    {"filepath", &transmit_wav_param.filepath, 0}};

// Schedule audio record task
esp_err_t API_AUDIO_Record(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Received record request");

    // Check if there is other instance of the task running
    TaskHandle_t audioRecordTaskHandle = xTaskGetHandle(audioRecordTaskName);

    esp_err_t ret = process_api_attributes(req, TAG, record_attributes, (sizeof(record_attributes) / sizeof(record_attributes[0])));

    // If processing attributes resulted in error we return early
    if (ret != ESP_OK)
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

// Schedule audio transmit wav task
esp_err_t API_AUDIO_TransmitWAV(httpd_req_t *req)
{
    // Check if there is other instance of the task running
    TaskHandle_t audioRecordTaskHandle = xTaskGetHandle(audioTransmitWAVTaskName);

    esp_err_t ret = process_api_attributes(req, TAG, transmit_wav_attributes, (sizeof(transmit_wav_attributes) / sizeof(transmit_wav_attributes[0])));

    ESP_LOGI(TAG, "Received audio play request for: %s", transmit_wav_param.filepath);

    // If processing attributes resulted in error we return early
    if (ret != ESP_OK)
    {
        return ret;
    }

    if (audioRecordTaskHandle == NULL)
    {
        xTaskCreate(TRANSMIT_Wav, audioTransmitWAVTaskName, 4096, &transmit_wav_param, RTOS_PRIORITY_HIGHEST, NULL);
        httpd_json_resp_send(req, HTTPD_200, "OK. Scheduled transmision of the WAV file.");
    }
    else
    {
        httpd_json_resp_send(req, HTTPD_500, "Transmit task is already running.");
    }

    return ESP_OK;
}