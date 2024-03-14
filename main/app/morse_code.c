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

#include "hardware/audio.h"

#define DOT_DURATION 200

static const char *TAG  = "APP/MORSE_CODE";
static const char *code = "- . ... - / .--- ..- ... - / .- / - . ... -";

void MORSE_CODE_TransmitOnce(const char *input, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        switch (input[i])
        {
        case '.':
            AUDIO_PlayTone(700, DOT_DURATION);
            break;
        case '-':
            AUDIO_PlayTone(700, DOT_DURATION * 3);
            break;
        }
        vTaskDelay(DOT_DURATION / portTICK_PERIOD_MS);
    }
}

void MORSE_CODE_Transmit(void *pvParameters)
{
    while (1)
    {
        MORSE_CODE_TransmitOnce(code, strlen(code));
        ESP_LOGI(TAG, "Transmitted: %s", code);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}