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
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <driver/ledc.h>

#include "led.h"
#include "settings.h"
#include "audio.h"

static const char *TAG = "HW/LED";

SemaphoreHandle_t gLedSemaphore;

// led fade complete callback function
static IRAM_ATTR bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    BaseType_t taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT)
    { // fade completed so we can give semaphore to release shared resource
        xSemaphoreGiveFromISR(gLedSemaphore, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}

// initialize PWM
static void init_pwm(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LED_PWM_RESOLUTION,
        .freq_hz = LED_PWM_FREQ_HZ,
        .speed_mode = LED_PWM_SPEED_MODE,
        .timer_num = LED_PWM_TIMER};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel = LED_PWM_CHANNEL,
        .duty = 0,
        .gpio_num = gSettings.gpio.status_led,
        .speed_mode = LED_PWM_SPEED_MODE,
        .timer_sel = LED_PWM_TIMER};

    ledc_channel_config(&ledc_channel);
    // initialize fade function
    ledc_fade_func_install(0);

    // initialize fade finished callback function
    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event};

    ledc_cb_register(LED_PWM_SPEED_MODE, LED_PWM_CHANNEL, &callbacks, NULL);
}

static void init_semaphore(void)
{
    gLedSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gLedSemaphore);
}

/// @brief Fade the LED
/// @param target_brightness target brightness (0-100)%
/// @param time_ms duration of the fade transition in ms
/// @param wait_to_finish determines whether it should wait (block) until fade is complete
/// @return
esp_err_t LED_Fade(uint8_t target_brightness, int time_ms, bool wait_to_finish)
{
    if (target_brightness > 100)
    {
        ESP_LOGE(TAG, "Target brightness %d exceeds max of %d", target_brightness, 100);
        return ESP_FAIL;
    }

    if (time_ms > LED_FADE_TIME_MAX)
    {
        ESP_LOGE(TAG, "Fade time %d exceeds LED_FADE_TIME_MAX of %d", time_ms, LED_FADE_TIME_MAX);
        return ESP_FAIL;
    }

    if (xSemaphoreTake(gLedSemaphore, LED_FADE_TIME_MAX / portTICK_PERIOD_MS) == pdFALSE)
    {
        // Fade in progress
        ESP_LOGW(TAG, "Overlapping fade operations detected.");
        return ESP_FAIL;
    }

    // TODO: Replace LED_VALUE_MAX with max brightness from settings
    int target_duty = LED_VALUE_MAX * target_brightness / 100;

    ledc_fade_mode_t fade_mode = wait_to_finish ? LEDC_FADE_WAIT_DONE : LEDC_FADE_NO_WAIT;

    ledc_set_fade_with_time(LED_PWM_SPEED_MODE, LED_PWM_CHANNEL, target_duty, time_ms);
    ledc_fade_start(LED_PWM_SPEED_MODE, LED_PWM_CHANNEL, fade_mode);

    return ESP_OK;
}


void LED_Init(void)
{
    init_semaphore();

    init_pwm();

    LED_Fade(100, LED_FADE_FAST, false);
    LED_Fade(0, LED_FADE_FASTEST, false);
}

// Show status of the device via LED
void LED_Status(void *pvParameters)
{
    while (1)
    {

        // TODO: This uses to big delays, so the LED doesn't immediatelly reflect state changes
        // switch (gAudioState)
        // {
        // case AUDIO_TRANSMITTING:
        //     gpio_set_level(gSettings.gpio.status_led, 1);
        //     vTaskDelay(100 / portTICK_PERIOD_MS);
        //     break;
        // default:
        //     gpio_set_level(gSettings.gpio.status_led, 0);
        //     vTaskDelay(LED_OFF_MS / portTICK_PERIOD_MS);

        //     gpio_set_level(gSettings.gpio.status_led, 1);
        //     vTaskDelay(LED_ON_MS / portTICK_PERIOD_MS);

        //     break;
        // }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
