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

#include <math.h>

#include "filter.h"
#include "helper/misc.h"

/// @brief Calculate and set parameters for the Butterworth filter
/// @param filter pointer to filter
/// @param frequency filter cutoff frequency in Hz
/// @param sampleRate sample rate in Hz
/// @param passType type of filter
/// @param resonance resonance of the filter
void FILTER_Init(FILTER_ButterworthFilter_t *filter, float frequency, int sampleRate, FILTER_PassType_t passType, float resonance) {
    switch (passType) {
        case Lowpass:
            filter->c = 1.0f / (float)tan(CONST_PI * frequency / sampleRate);
            filter->a1 = 1.0f / (1.0f + resonance * filter->c + filter->c * filter->c);
            filter->a2 = 2.0f * filter->a1;
            filter->a3 = filter->a1;
            filter->b1 = 2.0f * (1.0f - filter->c * filter->c) * filter->a1;
            filter->b2 = (1.0f - resonance * filter->c + filter->c * filter->c) * filter->a1;
            break;
        case Highpass:
            filter->c = (float)tan(CONST_PI * frequency / sampleRate);
            filter->a1 = 1.0f / (1.0f + resonance * filter->c + filter->c * filter->c);
            filter->a2 = -2.0f * filter->a1;
            filter->a3 = filter->a1;
            filter->b1 = 2.0f * (filter->c * filter->c - 1.0f) * filter->a1;
            filter->b2 = (1.0f - resonance * filter->c + filter->c * filter->c) * filter->a1;
            break;
    }
}

/// @brief Update filter and output filtered data
/// @param filter  pointer to filter
/// @param newInput data input to be filtered
/// @return filtered data output
float FILTER_Update(FILTER_ButterworthFilter_t *filter, float newInput) {
    float newOutput = filter->a1 * newInput + filter->a2 * filter->inputHistory[0] + filter->a3 * filter->inputHistory[1] - filter->b1 * filter->outputHistory[0] - filter->b2 * filter->outputHistory[1];
    filter->inputHistory[1] = filter->inputHistory[0];
    filter->inputHistory[0] = newInput;
    filter->outputHistory[1] = filter->outputHistory[0];
    filter->outputHistory[0] = newOutput;
    return newOutput;
}