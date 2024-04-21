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

// Define audio input max buffer size
#define AUDIO_INPUT_MAX_BUFF_SIZE 1024

// Define chunk size for audio input we process at a time
#define AUDIO_INPUT_CHUNK_SIZE 256

// Define audio input sampling frequency in Hz
#define AUDIO_INPUT_SAMPLE_FREQ 16000

// Define ADC conv mode
#define AUDIO_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1

// Define ADC output type
#define AUDIO_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1

// Define ADC input attenuation
#define AUDIO_ADC_ATTEN ADC_ATTEN_DB_12 // ADC_ATTEN_DB_12 allows to measure 0-3.3V

// Define ADC bit width
#define AUDIO_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define AUDIO_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define AUDIO_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define AUDIO_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define AUDIO_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define AUDIO_AFSK_TONE_MIN_FREQ 300
#define AUDIO_AFSK_TONE_MAX_FREQ 4000
#define AUDIO_AFSK_MIN_BAUD 50
#define AUDIO_AFSK_MAX_BAUD 2400

// I2S PDM TX clock io number (not used, but needs to be assigned)
#define AUDIO_PDM_TX_CLK_GPIO GPIO_NUM_21
#define AUDIO_BUFFER_SIZE 2048
// 16kHz seems fine with filtering
#define AUDIO_PDM_TX_FREQ_HZ 16000
// volume * AUDIO_VOLUME_MULTIPLIER = 1~32767, affects the volume
#define AUDIO_VOLUME_MULTIPLIER (320.0)
#define CONST_PI (3.1416f)

// At the same time we can either listen to audio, or transmit audio.
// We cannot do both at the same time. Event bits are used to syncronize shared I2S0 resource.
typedef enum
{
    BIT_STOP_LISTEN = (1 << 0), // used to request the listen task to stop listening, needed for audio transmit
    BIT_STOPPED_LISTENING = (1 << 1), // used to indicate that listen task stopped listening
    BIT_DONE_TX = (1 << 2) // used to indicate that audio tx is done
} AudioEventBit_t;

typedef enum
{
    AUDIO_OFF,
    AUDIO_LISTENING,
    AUDIO_RECEIVING,
    AUDIO_TRANSMITTING,
    AUDIO_LAST
} AudioState_t;

extern AudioState_t gAudioState;

esp_err_t AUDIO_TransmitStart(void);
esp_err_t AUDIO_TransmitStop(void);
void AUDIO_Listen(void *pvParameters);
void AUDIO_PlayTone(uint16_t freq, uint16_t duration_ms);
void AUDIO_PlayAFSK(const uint8_t *data, size_t len, uint16_t baud, uint16_t zero_freq, uint16_t one_freq);
void AUDIO_Init(void);
void AUDIO_AdcStop();

#endif