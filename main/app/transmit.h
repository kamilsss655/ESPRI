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

#ifndef APP_TRANSMIT_H
#define APP_TRANSMIT_H

#include <stdint.h>

typedef struct
{
    const char *input;
    uint8_t len;
} TRANSMIT_MorseCodeParam_t;

typedef struct
{
    const char *input;
    uint8_t len;
    uint16_t baud;
    uint16_t zero_freq;
    uint16_t one_freq;
} TRANSMIT_AfskParam_t;

void TRANSMIT_MorseCode(void *pvParameters);
void TRANSMIT_Afsk(void *pvParameters);

#endif