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
#include <esp_log.h>

#include "led.h"
#include "settings.h"
#include "external/ws2812b/esp_ws28xx.h"

static const char *TAG = "HW/WS2812B";

CRGB *ws2812_buffer;

// Blinks LED
void WS2812B_Blink(void *pvParameters)
{
    ws28xx_init(gSettings.gpio.status_led, WS2812B, 1, &ws2812_buffer);

    while (1)
    {
        ESP_LOGI(TAG, "Turning on the LED");
        ws2812_buffer[0] = (CRGB){.r = 0, .g = 0, .b = 10};
        ws28xx_update();
        vTaskDelay(LED_ON_MS / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Turning off the LED");
        ws2812_buffer[0] = (CRGB){.r = 0, .g = 0, .b = 0};
        ws28xx_update();
        vTaskDelay(LED_OFF_MS / portTICK_PERIOD_MS);
    }
}
