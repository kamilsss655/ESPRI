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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#define LED_PIN 2
#define LED_DELAY_MS 2000
#define LED_TAG "LED" 

void led_blink(void *pvParams) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction (LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_PIN, 1);
        ESP_LOGI(LED_TAG, "Turning on the LED");  
        vTaskDelay(LED_DELAY_MS / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN, 0);
        ESP_LOGI(LED_TAG, "Turning off the LED");  
        vTaskDelay(LED_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void app_main() {
    xTaskCreate(led_blink, "led_blink", 4096, NULL, 10, NULL);
}
