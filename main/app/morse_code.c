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
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "morse_code.h"
#include "hardware/audio.h"
#include "settings.h"
#include "helper/rtos.h"

#define DOT_DURATION 200

static const char *TAG = "APP/MORSE_CODE";

SemaphoreHandle_t transmitSemaphore;

void MORSE_CODE_TransmitOnce(void *pvParameters)
{
    MORSE_CODE_TransmitOnceParam_t *param = (MORSE_CODE_TransmitOnceParam_t *)pvParameters;

    ESP_LOGI(TAG, "Transmitting: %s", param->input);

    for (uint8_t i = 0; i < param->len; i++)
    {
        switch (param->input[i])
        {
        case '.':
            AUDIO_PlayTone(700, DOT_DURATION);
            vTaskDelay(DOT_DURATION / portTICK_PERIOD_MS);
            break;
        case '-':
            AUDIO_PlayTone(700, DOT_DURATION * 3);
            vTaskDelay(DOT_DURATION / portTICK_PERIOD_MS);
            break;
        }
        vTaskDelay((DOT_DURATION * 3) / portTICK_PERIOD_MS);
    }

    // Indicate that we are done and can transmit again
    xSemaphoreGive(transmitSemaphore);

    // Delete self
    vTaskDelete(NULL);
}

void MORSE_CODE_Transmit(void *pvParameters)
{
    transmitSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(transmitSemaphore);

    while (1)
    {
        // Check if beacon is enabled
        if (gSettings.morse_code_beacon.enabled == false)
        {
            // Give control back to RTOS for 5s before we check again
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        // Check if we can re-schedule new transmission
        if (xSemaphoreTake(transmitSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
        {
            // Transmission in progress
            ESP_LOGW(TAG, "Overlapping transmissions. Increase morse_code_beacon.period_seconds setting.");
            continue;
        }

        uint32_t delay_in_ms = gSettings.morse_code_beacon.period_seconds * 1000;

        MORSE_CODE_TransmitOnceParam_t param = {
            .input = gSettings.morse_code_beacon.text,
            .len = strlen(gSettings.morse_code_beacon.text)};

        // Schedule transmit task
        xTaskCreate(MORSE_CODE_TransmitOnce, "MORSE_CODE_TransmitOnce", 4096, &param, RTOS_PRIORITY_MEDIUM, NULL);

        // Delay before re-scheduling attempt
        vTaskDelay(delay_in_ms / portTICK_PERIOD_MS);
    }
}