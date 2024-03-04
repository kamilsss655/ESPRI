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

#include "button.h"

SemaphoreHandle_t buttonSemaphore = NULL;

// button pressed interrupt function
void IRAM_ATTR BUTTON_isr_handler(void *arg)
{
    // give semaphore to indicate button was pressed
    xSemaphoreGive(buttonSemaphore);
}

// initialize button handling
void BUTTON_init()
{
    buttonSemaphore = xSemaphoreCreateBinary();

    gpio_reset_pin(BUTTON_GPIO_PIN);
    gpio_set_direction(BUTTON_GPIO_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(BUTTON_GPIO_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, BUTTON_isr_handler, NULL);
}

// monitor button pressed semaphore
void BUTTON_monitor(void *pvParameters)
{
    BUTTON_init();

    while (1)
    { // if semaphore was set then the button was pressed
        if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(BUTTON_TAG, "Button pressed.");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}