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

#ifndef DSP_AFSK_H
#define DSP_AFSK_H

#include <stdint.h>
#include <stdbool.h> 

typedef struct
{
    uint16_t  samples_since_transition; // counts samples since last transition
    uint16_t  samples_read;             // counts samples read
    uint16_t  samples_window;           // samples window determines how many samples are read per bit 
    int16_t   last_sample;              // holds value of the last read sample
    uint8_t   current_bit;              // current demodulated bit
    bool      transition_detected;      // flag to indicate transition detected
} AFSK_t;

void AFSK_Init(AFSK_t *afsk);
void AFSK_Demodulate(AFSK_t *afsk, int16_t sample);

#endif