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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define SYSTEM_INTEGER_TYPE uint32_t
#define SYSTEM_TOTAL_HEAP_SIZE 320000
// Define GPIO pin mask of all the available pins on the board
#define SYSTEM_GPIO_PIN_MASK ((1ULL << 13) | (1ULL << 15) | (1ULL << 2) | (1ULL << 0) | (1ULL << 4) | (1ULL << 16) | (1ULL << 17) | (1ULL << 5) | (1ULL << 18) | (1ULL << 23) | (1ULL << 19) | (1ULL << 22) | (1ULL << 34) | (1ULL << 35) | (1ULL << 32) | (1ULL << 33) | (1ULL << 25) | (1ULL << 26) | (1ULL << 27) | (1ULL << 14) | (1ULL << 12))

// Heap info
typedef struct
{
    SYSTEM_INTEGER_TYPE total;
    SYSTEM_INTEGER_TYPE free;
    SYSTEM_INTEGER_TYPE min_free;
} SYSTEM_HeapInfo_t;

// Storage info
typedef struct
{
    SYSTEM_INTEGER_TYPE total;
    SYSTEM_INTEGER_TYPE used;
} SYSTEM_StorageInfo_t;

// Global system info
typedef struct
{
    SYSTEM_HeapInfo_t    heap;    // memory
    SYSTEM_StorageInfo_t storage; // flash storage for SPIFFS
    SYSTEM_INTEGER_TYPE  uptime;  // in seconds
    char                 version[32];
} SYSTEM_Info_t;

extern SYSTEM_Info_t gSystemInfo;

void SYSTEM_InfoRefresh(void *pvParameters);
void SYSTEM_InfoInit(void);
void SYSTEM_DeepSleep(void);
void SYSTEM_Reboot(void);

#endif