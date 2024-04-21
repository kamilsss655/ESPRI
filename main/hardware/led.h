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

#ifndef HARDWARE_LED_H
#define HARDWARE_LED_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define LED_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define LED_PWM_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LED_PWM_CHANNEL    LEDC_CHANNEL_0
#define LED_PWM_TIMER      LEDC_TIMER_0
#define LED_PWM_FREQ_HZ    10000
#define LED_VALUE_MAX      (1 << LED_PWM_RESOLUTION) // max LED brightness
#define LED_VALUE_MIN      0 // min LED brightness
#define LED_FADE_TIME_MAX LED_FADE_SLOWEST // max time to wait (block) in ms for fade function to finish

// Fade transition times in ms
enum {
    LED_FADE_FASTEST = 50,
    LED_FADE_FAST    = 300,
    LED_FADE_SLOW    = 600,
    LED_FADE_SLOWEST = 1200 
};

// Global LED semaphore for shared resource allocation
extern SemaphoreHandle_t gLedSemaphore;

void LED_Status(void *pvParameters);
void LED_Init(void);
esp_err_t LED_Fade(uint8_t target_brightness, int time_ms, bool wait_to_finish);

#endif