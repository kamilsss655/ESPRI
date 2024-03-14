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

#ifndef HARDWARE_AUDIO_H
#define HARDWARE_AUDIO_H

#include <driver/i2s_pdm.h>

// I2S PDM TX clock io number (not used, but needs to be assigned)
#define AUDIO_PDM_TX_CLK_GPIO GPIO_NUM_21
#define AUDIO_BUFFER_SIZE     2048
// 16kHz seems fine with filtering
#define AUDIO_PDM_TX_FREQ_HZ  16000
// 1~32767, affects the volume
#define AUDIO_WAVE_AMPLITUDE  (1000.0)        
#define CONST_PI              (3.1416f)

i2s_chan_handle_t AUDIO_Init(void);
void AUDIO_PlayTone(uint16_t freq, uint16_t duration_ms);

#endif