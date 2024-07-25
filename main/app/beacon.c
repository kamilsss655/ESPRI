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
// #include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "settings.h"
#include "transmit.h"
#include "helper/rtos.h"
#include "morse.h"

static const char *TAG = "APP/BEACON";

void BEACON_Scheduler(void *pvParameters)
{
    uint32_t delay_in_ms = gSettings.beacon.delay_seconds * 1000;

    ESP_LOGI(TAG, "Scheduler started.");

    while (1)
    {
        switch (gSettings.beacon.mode)
        {
        case SETTINGS_BEACON_MODE_OFF:
            break;

        case SETTINGS_BEACON_MODE_AFSK:
            // Schedule transmit task
            TRANSMIT_AfskParam_t afsk_param = {
                .input = gSettings.beacon.morse_code.text,
                .len = strlen(gSettings.beacon.morse_code.text),
                .baud = gSettings.beacon.afsk.baud,
                .zero_freq = gSettings.beacon.afsk.zero_freq,
                .one_freq = gSettings.beacon.afsk.one_freq};

            xTaskCreate(TRANSMIT_Afsk, "TRANSMIT_Afsk", 4096, &afsk_param, RTOS_PRIORITY_HIGHEST, NULL);
            break;

        case SETTINGS_BEACON_MODE_MORSE_CODE:
            // Schedule transmit task
            TRANSMIT_MorseCodeParam_t morse_code_param = {
                .input = string2morse(gSettings.beacon.text),
                .len = strlen(string2morse(gSettings.beacon.text))};

            xTaskCreate(TRANSMIT_MorseCode, "TRANSMIT_MorseCode", 4096, &morse_code_param, RTOS_PRIORITY_HIGHEST, NULL);
            break;

        case SETTINGS_BEACON_MODE_WAV:
            // Schedule transmit task
            TRANSMIT_WavParam_t wav_param;
            strcpy(wav_param.filepath, gSettings.beacon.wav.filepath);
            xTaskCreate(TRANSMIT_Wav, "TRANSMIT_Wav", 4096, &wav_param, RTOS_PRIORITY_HIGHEST, NULL);
            break;
        }

        // Delay before re-scheduling attempt
        vTaskDelay(delay_in_ms / portTICK_PERIOD_MS);
    }
}