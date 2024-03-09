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

#include <stddef.h>
#include <esp_log.h>

#include "hardware/uart.h"
#include "external/printf/printf.h"

static const char *TAG = "APP/UVK5";

// Send message
void UVK5_SendMessage(char *message, size_t size)
{
    char buffer[size + 7];

    snprintf(buffer, sizeof(buffer), "SMS: %s\n", message);
    UART_Send((const char *)buffer, sizeof(buffer));
    ESP_LOGI(TAG, "Sent message: %s", message);
}