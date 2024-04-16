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

#include <freertos/FreeRTOS.h>
#include <esp_app_desc.h>
#include <esp_spiffs.h>
#include <string.h>
#include <stdint.h>

#include "system.h"

SYSTEM_Info_t gSystemInfo;

// Refresh gSystemInfo task
void SYSTEM_InfoRefresh(void *pvParameters)
{
    while (1)
    {

        gSystemInfo.heap.free = esp_get_free_heap_size();
        gSystemInfo.heap.min_free = esp_get_minimum_free_heap_size();
        gSystemInfo.uptime += 1;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void SYSTEM_InfoInit(void)
{
    // Version
    const esp_app_desc_t *app_desc;
    app_desc = esp_app_get_description();
    memcpy(gSystemInfo.version, app_desc->version, sizeof(gSystemInfo.version));

    // Total heap memory size
    gSystemInfo.heap.total = SYSTEM_TOTAL_HEAP_SIZE;

    // SPIFFS /storage size
    size_t storage_total_bytes;
    size_t storage_free_bytes;

    esp_spiffs_info(NULL, &storage_total_bytes, &storage_free_bytes);

    gSystemInfo.storage.total = (SYSTEM_INTEGER_TYPE)storage_total_bytes;
    gSystemInfo.storage.free = (SYSTEM_INTEGER_TYPE)storage_free_bytes;
}