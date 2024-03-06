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

#include "button.h"
#include "app/button.h"

static const char *TAG = "HW/BUTTON";

QueueHandle_t buttonQueue;

// button pressed interrupt function
static void IRAM_ATTR BUTTON_isr_handler(void *arg)
{
    // pin number of the button pressed
    int pinNumber = (uint8_t)arg;
    // send button pressed event to the button queue
    xQueueSendFromISR(buttonQueue, &pinNumber, NULL);
}

// initialize button handling
void BUTTON_init()
{
    buttonQueue = xQueueCreate(10, sizeof(uint8_t));

    gpio_reset_pin(BUTTON_GPIO_PIN);
    gpio_set_direction(BUTTON_GPIO_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(BUTTON_GPIO_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, BUTTON_isr_handler, (void *)BUTTON_GPIO_PIN);
}

// monitor button pressed queue
void BUTTON_monitor(void *pvParameters)
{
    BUTTON_Event_t buttonEvent;

    BUTTON_init();

    while (true)
    {
        if (xQueueReceive(buttonQueue, &buttonEvent.pin_number, portMAX_DELAY))
        {
            buttonEvent.type = BUTTON_PRESSED;

            ESP_LOGI(TAG, "GPIO %d was pressed.", buttonEvent.pin_number);

            // TODO: Disable interrupt for a while to debounce

            // Handle button event
            BUTTON_handle(buttonEvent);
        }
    }
}