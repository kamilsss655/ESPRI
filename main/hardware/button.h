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

#ifndef HARDWARE_BUTTON_H
#define HARDWARE_BUTTON_H

#include <stdint.h>

// Button event type
typedef enum
{
    BUTTON_PRESSED,
    BUTTON_HELD
} BUTTON_EventType_t;

// Button event type
typedef enum
{
    BUTTON_1 = 1
} BUTTON_Number_t;

// Button event
typedef struct
{
    BUTTON_EventType_t type;   // button event type
    BUTTON_Number_t    number; // button number
} BUTTON_Event_t;

void BUTTON_Monitor(void *pvParameters);
void BUTTON_Init();

#endif