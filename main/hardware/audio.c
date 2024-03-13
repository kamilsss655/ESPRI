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
#include <math.h>
#include <driver/i2s_pdm.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "audio.h"

static const char *TAG = "HW/AUDIO";

i2s_chan_handle_t tx_channel;

// Initialize audio
// Note: Keep wires/traces from PDM output pin as short as possible to minimalize interference
i2s_chan_handle_t AUDIO_Init(void)
{
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    
    tx_chan_cfg.auto_clear = true;
    
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_channel, NULL));

    i2s_pdm_tx_config_t pdm_tx_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(AUDIO_PDM_TX_FREQ_HZ),
        /* The data bit-width of PDM mode is fixed to 16 */
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = AUDIO_PDM_TX_CLK_GPIO,
            .dout = CONFIG_AUDIO_OUT_GPIO,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_pdm_tx_mode(tx_channel, &pdm_tx_cfg));

    ESP_LOGI(TAG, "Audio output initialized on pin: %d", CONFIG_AUDIO_OUT_GPIO);

    return tx_channel;
}

// Play tone
void AUDIO_PlayTone(uint16_t freq, uint16_t duration_ms)
{
    size_t w_bytes = 0;

    // Allocate temp buffer
    int16_t *w_buf = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf);

    uint32_t duration_sine = (AUDIO_PDM_TX_FREQ_HZ / (float)freq) + 0.5;

    /* Generate the tone buffer */
    // Single sinewave

    for (int i = 0; i < duration_sine; i++)
    {
        w_buf[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine)) * AUDIO_WAVE_AMPLITUDE);
    }

    // Multiply single sinewave to desired duration

    uint32_t duration_i2s = duration_ms * AUDIO_PDM_TX_FREQ_HZ / 1000;

    // Turn on PDM output
    ESP_ERROR_CHECK(i2s_channel_enable(tx_channel));

    for (int tot_bytes = 0; tot_bytes < duration_i2s; tot_bytes += w_bytes)
    {
        /* Play the tone */
        if (i2s_channel_write(tx_channel, w_buf, duration_sine * sizeof(int16_t), &w_bytes, 1000) != ESP_OK)
        {
            printf("Write Task: i2s write failed\n");
        }
    }

     // Turn off PDM output
    ESP_ERROR_CHECK(i2s_channel_disable(tx_channel));

    // Deallocate temp buffer
    free(w_buf);
}