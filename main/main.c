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

#include "helper/rtos.h"
#include "hardware/led.h"
#include "hardware/button.h"
#include "drivers/uart.h"

void app_main()
{
    // Create LED_blink task
    xTaskCreate(LED_blink, "LED_blink", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);
    // Create button monitor task
    xTaskCreate(BUTTON_monitor, "BUTTON_monitor", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);
    UART_run();
}
