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

typedef enum {
    Lowpass,
    Highpass
} FILTER_PassType_t;

typedef struct {
    float c, a1, a2, a3, b1, b2;
    float inputHistory[2];
    float outputHistory[2];
} FILTER_ButterworthFilter_t;

void FILTER_Init(FILTER_ButterworthFilter_t *filter, float frequency, int sampleRate, FILTER_PassType_t passType, float resonance);
float FILTER_Update(FILTER_ButterworthFilter_t *filter, float newInput);