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

#include "led.h"

static const char* TAG = "LED";

// Blinks LED
void LED_blink(void *pvParameters)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1)
    {
        ESP_LOGI(TAG, "Turning on the LED");
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(LED_DELAY_MS / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Turning off the LED");
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(LED_DELAY_MS / portTICK_PERIOD_MS);
    }
}
