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
#include "system.h"
#include "app/beacon.h"
#include "helper/rtos.h"
#include "hardware/audio.h"
#include "hardware/button.h"
#include "hardware/uart.h"
#include "web/handlers/websocket.h"
#include "hardware/led.h"

void app_main()
{
    // Init board
    BOARD_Init();

    // Create LED_Status task
    xTaskCreate(LED_Status, "LED_Status", 2048, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Create button monitor task
    xTaskCreate(BUTTON_Monitor, "BUTTON_Monitor", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Create UART monitor task
    xTaskCreate(UART_Monitor, "UART_Monitor", 4096, NULL, RTOS_PRIORITY_MEDIUM, NULL);

    // Audio listen task
    xTaskCreate(AUDIO_Listen, "AUDIO_Listen", 4096, NULL, RTOS_PRIORITY_HIGHEST, NULL);

    // Audio input process task
    xTaskCreate(AUDIO_AudioInputProcess, "AUDIO_AudioInputProcess", 4096, NULL, RTOS_PRIORITY_HIGHEST, NULL);

    // Audio watchdog    
    xTaskCreate(AUDIO_Watchdog, "AUDIO_Watchdog", 2048, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Audio squelch control
    xTaskCreate(AUDIO_SquelchControl, "AUDIO_SquelchControl", 4096, NULL, RTOS_PRIORITY_MEDIUM, NULL);

    // Create Morse code transmit task
    xTaskCreate(BEACON_Scheduler, "BEACON_Scheduler", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);

    // Create websocket ping task
    xTaskCreate(WEBSOCKET_Ping, "WEBSOCKET_Ping", 4096, NULL, RTOS_PRIORITY_IDLE, NULL);
    
    // Create system info refresh task
    xTaskCreate(SYSTEM_InfoRefresh, "SYSTEM_InfoRefresh", 2048, NULL, RTOS_PRIORITY_IDLE, NULL);
    
}
