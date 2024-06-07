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
#include <esp_system.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <esp_app_desc.h>
#include <esp_spiffs.h>
#include <driver/gpio.h>

#include "system.h"
#include "settings.h"
#include "hardware/led.h"
#include "hardware/sd.h"

static const char *TAG = "SYSTEM";

SYSTEM_Info_t gSystemInfo;

// Refresh gSystemInfo task
void SYSTEM_InfoRefresh(void *pvParameters)
{
    while (1)
    {
        // Memory usage
        gSystemInfo.heap.free = esp_get_free_heap_size();
        gSystemInfo.heap.min_free = esp_get_minimum_free_heap_size();

        // Uptick uptime counter
        gSystemInfo.uptime += 1;

        // SPIFFS /storage size
        size_t storage_total_bytes;
        size_t storage_used_bytes;

        esp_spiffs_info(NULL, &storage_total_bytes, &storage_used_bytes);

        gSystemInfo.storage.total = (SYSTEM_INTEGER_TYPE)storage_total_bytes;
        gSystemInfo.storage.used = (SYSTEM_INTEGER_TYPE)storage_used_bytes;

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
}

// Gracefully take care of all the running tasks, gpio, spiffs before shutdown
void before_shutdown(void)
{
    // We forcefully take semaphore in case other tasks are using the LED (i.e PTT)
    xSemaphoreGive(gLedSemaphore);
    // Fade off the LED
    LED_Fade(LED_BRIGHTNESS_OFF, LED_TIME_FASTEST, true);
    // Indicate shutdown to user
    LED_Blink(LED_BLINK_ON_OFF, LED_TIME_FASTEST, LED_BRIGHTNESS_MAX);
    // Take LED semaphore to prevent other tasks interacting with the LED
    xSemaphoreTake(gLedSemaphore, LED_TIME_MAX / portTICK_PERIOD_MS);
    // Shutdown SD card
    SD_Shutdown();

    // Reset GPIO pins to prevent LED or PTT on during deep sleep etc.
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = SYSTEM_GPIO_PIN_MASK, // Set all GPIO pins
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };

    gpio_config(&io_conf);

    // Unregister SPIFFS
    esp_vfs_spiffs_unregister(NULL);
}

void SYSTEM_DeepSleep(void)
{
    ESP_LOGI(TAG, "Deep sleep");

    before_shutdown();

    // Enable wake-up with touch button
    ESP_ERROR_CHECK(esp_sleep_enable_touchpad_wakeup());
    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));

    esp_deep_sleep_start();
}

void SYSTEM_Reboot(void)
{
    ESP_LOGI(TAG, "Reboot");

    before_shutdown();

    esp_restart();
}

// Called upon first system boot (after flashing new firmware)
void SYSTEM_FirstBoot(void)
{
    ESP_LOGI(TAG, "First boot");

    SETTINGS_FactoryReset(true);
}