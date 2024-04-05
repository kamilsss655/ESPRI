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
            // TODO: To be implemented
            break;

        case SETTINGS_BEACON_MODE_MORSE_CODE:
            // Schedule transmit task
            TRANSMIT_MorseCodeParam_t param = {
                .input = gSettings.beacon.text,
                .len = strlen(gSettings.beacon.text)};

            xTaskCreate(TRANSMIT_MorseCode, "TRANSMIT_MorseCode", 4096, &param, RTOS_PRIORITY_MEDIUM, NULL);
            break;
        }

        // Delay before re-scheduling attempt
        vTaskDelay(delay_in_ms / portTICK_PERIOD_MS);
    }
}