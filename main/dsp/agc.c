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

#include <stdint.h>
#include <stdlib.h>

#include "agc.h"

// Initialize AGC
void AGC_Init(AGC_t *agc, uint8_t initial_gain)
{
    agc->current_gain = initial_gain;
    agc->samples_count = 0;
    agc->max_input_value = 0;
}

// AGC calculate and apply gain
// It keeps track of the maximum sample value per series of samples
// and adjusts the gain so it stays within desired range
// additionally it prevents clipping
int16_t AGC_Update(AGC_t *agc, int16_t value)
{
    int16_t amplified_value = abs(value) * agc->current_gain;

    if (amplified_value > agc->max_input_value)
        agc->max_input_value = amplified_value;

    if (agc->samples_count < DSP_AGC_SAMPLES_PER_STEP)
    {
        agc->samples_count++;
    }
    else
    {
        int16_t error = DSP_AGC_TARGET_VALUE - agc->max_input_value;

        if (error > 0)
        {
            if (agc->current_gain <= DSP_AGC_MAX_GAIN - DSP_AGC_STEP)
            {
                agc->current_gain += DSP_AGC_STEP;
            }
            else
            {
                agc->current_gain = DSP_AGC_MAX_GAIN;
            }
        }
        else
        {
            if (agc->current_gain >= DSP_AGC_MIN_GAIN + DSP_AGC_STEP)
            {
                agc->current_gain -= DSP_AGC_STEP;
            }
            else
            {
                agc->current_gain = DSP_AGC_MIN_GAIN;
            }
        }

        agc->max_input_value = 0;
        agc->samples_count = 0;
    }

    // Prevent clipping
    if (value > INT16_MAX / agc->current_gain)
    {
        return INT16_MAX;
    }
    else if (value < INT16_MIN / agc->current_gain)
    {
        return INT16_MIN;
    }
    else
    {
        // Otherwise return amplified value
        return value *= agc->current_gain;
    }
}