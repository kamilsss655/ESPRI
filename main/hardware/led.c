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
#include <driver/gpio.h>
#include <esp_log.h>

#include "led.h"
#include "settings.h"
#include "audio.h"

static const char *TAG = "HW/LED";

void LED_Init(void)
{
    gpio_reset_pin(gSettings.gpio.status_led);
    gpio_set_direction(gSettings.gpio.status_led, GPIO_MODE_OUTPUT);
    
    ESP_LOGI(TAG, "Initialized at pin: %d", gSettings.gpio.status_led);

    // Blink once to indicate boot
    gpio_set_level(gSettings.gpio.status_led, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(gSettings.gpio.status_led, 0);

}

// Show status of the device via LED
void LED_Status(void *pvParameters)
{
    while (1)
    {

        // TODO: This uses to big delays, so the LED doesn't immediatelly reflect state changes
        switch (gAudioState)
        {
        case AUDIO_TRANSMITTING:
            gpio_set_level(gSettings.gpio.status_led, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;
        default:
            gpio_set_level(gSettings.gpio.status_led, 0);
            vTaskDelay(LED_OFF_MS / portTICK_PERIOD_MS);

            gpio_set_level(gSettings.gpio.status_led, 1);
            vTaskDelay(LED_ON_MS / portTICK_PERIOD_MS);

            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
