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

#include <esp_log.h>

#include "afsk.h"
#include "hardware/audio.h"

static const char *TAG = "DSP/AFSK";

void AFSK_Init(AFSK_t *afsk)
{
    afsk->last_sample = 0;
    afsk->samples_read = 0;
    afsk->samples_since_transition = 0;
    afsk->samples_window = AUDIO_AFSK_SAMPLESPERBIT;
    afsk->transition_detected = false;
}
void AFSK_Demodulate(AFSK_t *afsk, int16_t sample)
{
    // increment samples read counter
    afsk->samples_read++;

    if (afsk->transition_detected == false)
    {
        if (sample == afsk->last_sample)
        {
            afsk->samples_since_transition++;
        }
        else
        {
            afsk->transition_detected = true;
        }
    }

    // memorize sample
    afsk->last_sample = sample;

    // We reached the samples window, so now we demodulate the bit and adjust the next samples window size
    // based on where the last transition was
    // this way we constantly sync with the incoming bitstream (even if clocks between devices do not match)
    if (afsk->samples_read >= afsk->samples_window)
    {
        if (afsk->transition_detected)
        {
            // if the transition was past the half of the window
            if (afsk->samples_since_transition > (afsk->samples_window / 2))
            {
                // we increase the size of the window, in effect slowing down the sample rate
                afsk->samples_window = AUDIO_AFSK_SAMPLESPERBIT + AUDIO_AFSK_SAMPLES_STEP;
            }
            // if the transition was before half of the window
            else
            {
                // we decrease the size of the window, in effect speeding up the sample rate
                afsk->samples_window = AUDIO_AFSK_SAMPLESPERBIT - AUDIO_AFSK_SAMPLES_STEP;
            }
        }
        else
        { // if there were no transitions detected we keep the standard window size
            afsk->samples_window = AUDIO_AFSK_SAMPLESPERBIT;
        }
        // current bit is the first bit in the processing window
        afsk->current_bit = sample > 0 ? 1 : 0;

        // bitshift output_byte by 1 to make space for new bit
        // output_byte <<= 1;

        // if current bit is 1 then set least significant bit of output_byte to 1 otherwise it will be 0
        if (afsk->current_bit == 1)
        {
            // output_byte |= 1;
            ESP_LOGI(TAG, "1");
        }
        else
        {
            ESP_LOGI(TAG, "0");
        }

        // reset
        afsk->samples_read = 0;
        afsk->samples_since_transition = 0;
        afsk->transition_detected = false;
    }
}