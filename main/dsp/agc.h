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

#ifndef DSP_AGC_H
#define DSP_AGC_H

#include <stdint.h>

#define DSP_AGC_MIN_GAIN 1
#define DSP_AGC_MAX_GAIN 255
#define DSP_AGC_MAX_VAL 32767  // max value for int16_t
#define DSP_AGC_MIN_VAL -32767 // min value for int16_t
// Determine target sample value for the gain function
#define DSP_AGC_TARGET_VALUE DSP_AGC_MAX_VAL * 6 / 10
// Determine gain step
#define DSP_AGC_STEP 10
// Determine how many samples to take before changing gain
#define DSP_AGC_SAMPLES_PER_STEP 16000

typedef struct
{
    uint8_t current_gain;    // current gain
    uint16_t samples_count;  // samples counter
    int16_t max_input_value; // max value from a series of samples
} AGC_t;

void AGC_Init(AGC_t *agc, uint8_t initial_gain);
int16_t AGC_Update(AGC_t *agc, int16_t value);

#endif