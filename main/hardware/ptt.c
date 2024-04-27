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

#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "settings.h"
#include "led.h"

static const char *TAG = "HW/PTT";

// Initialize PTT
esp_err_t PTT_Init(void)
{
    // Enable PTT GPIO Pin
    ESP_ERROR_CHECK(gpio_reset_pin(gSettings.gpio.ptt));
    ESP_ERROR_CHECK(gpio_set_direction(gSettings.gpio.ptt, GPIO_MODE_OUTPUT));

    ESP_LOGI(TAG, "PTT output initialized on pin: %d", gSettings.gpio.ptt);
    ESP_LOGI(TAG, "PTT on delay: %dms", CONFIG_PTT_ON_DELAY);
    ESP_LOGI(TAG, "PTT off delay: %dms", CONFIG_PTT_OFF_DELAY);

    return ESP_OK;
}

// Press PTT
void PTT_Press()
{
    gpio_set_level(gSettings.gpio.ptt, 1);
    ESP_LOGI(TAG, "PTT pressed!");

    // Wait a bit with PTT pressed before we output audio
    vTaskDelay(CONFIG_PTT_ON_DELAY / portTICK_PERIOD_MS);
}

// Release PTT
void PTT_Release()
{
    // Wait a bit in silence before we release PTT
    vTaskDelay(CONFIG_PTT_OFF_DELAY / portTICK_PERIOD_MS);

    gpio_set_level(gSettings.gpio.ptt, 0);
    ESP_LOGI(TAG, "PTT released!");
}