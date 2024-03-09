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

#include "board.h"
#include "helper/rtos.h"
#include "hardware/led.h"
#include "hardware/button.h"
#include "hardware/uart.h"

void app_main()
{
    // Init board
    BOARD_init();

    // TODO: Reorganize code as such:
    // Init tasks
    // TASKS_init();

    // Create LED_Blink task
    xTaskCreate(LED_Blink, "LED_Blink", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Create button monitor task
    xTaskCreate(BUTTON_Monitor, "BUTTON_Monitor", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Create UART monitor task
    xTaskCreate(UART_Monitor, "UART_Monitor", 4096, NULL, RTOS_PRIORITY_MEDIUM, NULL);
}
