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

#include <string.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <esp_app_desc.h>
#include <esp_spiffs.h>
#include <driver/gpio.h>

#include "system.h"
#include "settings.h"

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
    size_t storage_used_bytes;

    esp_spiffs_info(NULL, &storage_total_bytes, &storage_used_bytes);

    gSystemInfo.storage.total = (SYSTEM_INTEGER_TYPE)storage_total_bytes;
    gSystemInfo.storage.used = (SYSTEM_INTEGER_TYPE)storage_used_bytes;
}

// Gracefully take care of all the running tasks, gpio, spiffs before shutdown
void SYSTEM_Shutdown(void)
{
    // Reset GPIO pins to prevent LED or PTT on during deep sleep etc.
    gpio_reset_pin(gSettings.gpio.status_led);
    gpio_reset_pin(gSettings.gpio.audio_out);
    gpio_reset_pin(gSettings.gpio.ptt);

    // Unregister SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}