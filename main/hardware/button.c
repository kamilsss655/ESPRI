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
#include "driver/touch_pad.h"
#include <esp_log.h>

#include "button.h"
#include "app/button.h"

static const char *TAG = "HW/BUTTON";

QueueHandle_t buttonQueue;

// button pressed interrupt function
// static void IRAM_ATTR BUTTON_isr_handler(void *arg)
// {
//     // pin number of the button pressed
//     uint8_t pinNumber = (int)arg;
//     // send button pressed event to the button queue
//     xQueueSendFromISR(buttonQueue, &pinNumber, NULL);
// }

static void calibrate_touch_pad(touch_pad_t pad)
{
    int avg = 0;
    const size_t calibration_count = 128;

    for (int i = 0; i < calibration_count; ++i)
    {
        uint16_t val;
        touch_pad_read(pad, &val);
        avg += val;
    }
    avg /= calibration_count;
    const int min_reading = 300;
    if (avg < min_reading)
    {
        ESP_LOGE(TAG, "Touch pad #%d average reading is too low: %d (expecting at least %d). "
               "Not using for deep sleep wakeup.",
               pad, avg, min_reading);
        touch_pad_config(pad, 0);
    }
    else
    {
        int threshold = avg - 100;
        ESP_LOGI(TAG, "Calibrated touch pad #%d. Count average: %d, wakeup threshold set to %d.", pad, avg, threshold);
        touch_pad_config(pad, threshold);
    }
}

// Initialize touch pad
void BUTTON_InitTouchPad(touch_pad_t num)
{
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    ESP_ERROR_CHECK(touch_pad_init());
    // If use touch pad wake up, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
  
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    // init RTC IO and mode for touch pad.
    touch_pad_config(num, 400);
    // touch_pad_config(num, TOUCH_THRESH_NO_USE);
    calibrate_touch_pad(num);
}

// initialize button handling
void BUTTON_Init()
{
    buttonQueue = xQueueCreate(10, sizeof(uint8_t));

    BUTTON_InitTouchPad(CONFIG_TOUCH_BUTTON_1_NUMBER);
}

// monitor button pressed queue
void BUTTON_Monitor(void *pvParameters)
{
    BUTTON_Event_t buttonEvent;

    while (true)
    {
        if (xQueueReceive(buttonQueue, &buttonEvent.pin_number, portMAX_DELAY))
        {
            buttonEvent.type = BUTTON_PRESSED;

            ESP_LOGI(TAG, "GPIO %d was pressed.", buttonEvent.pin_number);

            // Handle button event
            BUTTON_Handle(buttonEvent);
        }
    }
}