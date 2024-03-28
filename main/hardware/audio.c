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
#include <string.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/i2s_pdm.h>
#include <esp_adc/adc_continuous.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "audio.h"
#include "settings.h"
#include "helper/rtos.h"

static const char *TAG = "HW/AUDIO";

static TaskHandle_t audioListenTaskHandle;

EventGroupHandle_t audioEventGroup;

adc_unit_t audioAdcUnit;

// AudioState_t gAudioState;

// I2S handle to transmit audio
i2s_chan_handle_t tx_channel;
// I2S handle to receive/listen audio
static adc_continuous_handle_t rx_channel;

// Interupt callback called when ADC finishes one portion of ADC conversions
static bool IRAM_ATTR adc_conv_done_callback(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(audioListenTaskHandle, &mustYield);

    return (mustYield == pdTRUE);
}

// Initialize ADC
static void AUDIO_AdcInit()
{
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = AUDIO_INPUT_MAX_BUFF_SIZE,
        .conv_frame_size = AUDIO_INPUT_CHUNK_SIZE,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &rx_channel));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = AUDIO_INPUT_SAMPLE_FREQ,
        .conv_mode = AUDIO_ADC_CONV_MODE,
        .format = AUDIO_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};

    static adc_channel_t audioAdcChannel;

    // set adc channel and unit based on audio input GPIO number
    adc_continuous_io_to_channel(AUDIO_INPUT_PIN, &audioAdcUnit, &audioAdcChannel);

    dig_cfg.pattern_num = 1;

    adc_pattern[0].atten = AUDIO_ADC_ATTEN;
    adc_pattern[0].atten = AUDIO_ADC_ATTEN;
    adc_pattern[0].channel = audioAdcChannel;
    adc_pattern[0].unit = audioAdcUnit;
    adc_pattern[0].bit_width = AUDIO_ADC_BIT_WIDTH;

    dig_cfg.adc_pattern = adc_pattern;

    ESP_ERROR_CHECK(adc_continuous_config(rx_channel, &dig_cfg));

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = adc_conv_done_callback,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(rx_channel, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(rx_channel));

    ESP_LOGI(TAG, "ADC initialized.");
    ESP_LOGI(TAG, "ADC attenuation: %" PRIx8, dig_cfg.adc_pattern[0].atten);
    ESP_LOGI(TAG, "ADC channel: %" PRIx8, dig_cfg.adc_pattern[0].channel);
}

// Stop ADC
void AUDIO_AdcStop()
{
    adc_continuous_stop(rx_channel);
    adc_continuous_deinit(rx_channel);
}

// Task listening to incoming audio on ADC port
void AUDIO_Listen(void *pvParameters)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[AUDIO_INPUT_CHUNK_SIZE] = {0};
    memset(result, 0xcc, AUDIO_INPUT_CHUNK_SIZE);
    audioListenTaskHandle = xTaskGetCurrentTaskHandle();
    char unit[] = "#unit";
    EventBits_t audioEventGroupBits;

    AUDIO_AdcInit();

    while (1)
    {
        // Read event bits
        audioEventGroupBits = xEventGroupGetBits(audioEventGroup);

        if ((audioEventGroupBits & BIT_STOP_LISTEN) != 0)
        { // something requested that we stop listening
            AUDIO_AdcStop();
            ESP_LOGI(TAG, "Stopped ADC.");
            // clear bit
            xEventGroupClearBits(audioEventGroup, BIT_STOP_LISTEN);
            // indicate that we stopped listening
            xEventGroupSetBits(audioEventGroup, BIT_STOPPED_LISTENING);
        }
        else if ((audioEventGroupBits & BIT_STOPPED_LISTENING) != 0)
        { // we idle here, until external process turns of this bit
            ESP_LOGI(TAG, "Waiting for TX to end.");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else if ((audioEventGroupBits & BIT_DONE_TX) != 0)
        {
            ESP_LOGI(TAG, "Detected TX end, re-initializing listening.");
            // re-init adc
            AUDIO_AdcInit();
            xEventGroupClearBits(audioEventGroup, BIT_DONE_TX);
        }
        else // if no bits are set do listen (default state)
        {
            // Block until we receive notification from the interupt that data is available
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ret = adc_continuous_read(rx_channel, result, AUDIO_INPUT_CHUNK_SIZE, &ret_num, 0);

            if (ret == ESP_OK)
            {
                // ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
                {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                    uint32_t chan_num = AUDIO_ADC_GET_CHANNEL(p);
                    uint32_t data = AUDIO_ADC_GET_DATA(p);
                    /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                    if (chan_num < SOC_ADC_CHANNEL_NUM(audioAdcUnit))
                    {
                        ESP_LOGI(TAG, "Unit: %s, Channel: %" PRIu32 ", Value: %" PRIx32, unit, chan_num, data);
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Invalid data [%s_%" PRIu32 "_%" PRIx32 "]", unit, chan_num, data);
                    }
                }
                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */
                vTaskDelay(1);
            }
            else if (ret == ESP_ERR_TIMEOUT)
            {
                // ESP_ERR_TIMEOUT means there is no available data to read, so we idle
                vTaskDelay(10);
            }
        }
    }
}

// Start audio transmit
// This will stop audio listen so we can use I2S0 peripheral for transmit
// Note: Keep wires/traces from PDM output pin as short as possible to minimalize interference
i2s_chan_handle_t AUDIO_TransmitStart(void)
{
    EventBits_t audioEventGroupBits;

    // request that we stop listening
    xEventGroupSetBits(audioEventGroup, BIT_STOP_LISTEN);

    // block until listening stopped
    do
    {
        audioEventGroupBits = xEventGroupGetBits(audioEventGroup);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while ((audioEventGroupBits & BIT_STOPPED_LISTENING) != BIT_STOPPED_LISTENING);

    ESP_LOGI(TAG, "Transmit starting");

    // Configure I2S0 peripheral for audio transmit

    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    tx_chan_cfg.auto_clear = true;

    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_channel, NULL));

    i2s_pdm_tx_config_t pdm_tx_cfg = {
        .clk_cfg = I2S_PDM_TX_CLK_DEFAULT_CONFIG(AUDIO_PDM_TX_FREQ_HZ),
        /* The data bit-width of PDM mode is fixed to 16 */
        .slot_cfg = I2S_PDM_TX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .clk = AUDIO_PDM_TX_CLK_GPIO,
            .dout = gSettings.gpio.audio_out,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_pdm_tx_mode(tx_channel, &pdm_tx_cfg));

    ESP_LOGI(TAG, "Audio output initialized on pin: %d", gSettings.gpio.audio_out);

    return tx_channel;
}

// Stop audio transmit
esp_err_t AUDIO_TransmitStop(void)
{
    ESP_LOGI(TAG, "Transmit stop");

    // Release I2S0 peripheral
    // TODO: This causes runtime error if channel was already disabled
    // we should probably check if it was disabled before disabling it
    i2s_channel_disable(tx_channel);
    i2s_del_channel(tx_channel);

    // Indicate that audio transmit is finished
    xEventGroupClearBits(audioEventGroup, BIT_STOPPED_LISTENING);
    xEventGroupSetBits(audioEventGroup, BIT_DONE_TX);

    return ESP_OK;
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

// Play AFSK coded data
// TODO: Works but needs refactor to be cleaner
void AUDIO_PlayAFSK(uint8_t *data, size_t len)
{
    int zero_freq = 800;
    int one_freq = 1600;

    int duration_ms = 30; // 33 baud

    size_t w_bytes = 0;

    // Allocate temp buffer one
    int16_t *w_buf_one = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf_one);

    // Allocate temp buffer zero
    int16_t *w_buf_zero = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf_zero);

    uint32_t duration_sine_zero = (AUDIO_PDM_TX_FREQ_HZ / (float)zero_freq) + 0.5;
    uint32_t duration_sine_one = (AUDIO_PDM_TX_FREQ_HZ / (float)one_freq) + 0.5;

    /* Generate the tone buffer */
    // Single sinewave zero

    for (int i = 0; i < duration_sine_zero; i++)
    {
        w_buf_zero[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine_zero)) * AUDIO_WAVE_AMPLITUDE);
    }

    /* Generate the tone buffer */
    // Single sinewave one

    for (int i = 0; i < duration_sine_one; i++)
    {
        w_buf_one[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine_one)) * AUDIO_WAVE_AMPLITUDE);
    }

    // Multiply single sinewave to desired duration

    uint32_t duration_i2s = duration_ms * AUDIO_PDM_TX_FREQ_HZ / 1000;

    // for (int i = 0; i < sizeof(data)/sizeof(uint8_t); i++)
    //     for (int bit = 7; bit >= 0; bit--)
    //     {
    //         if ((data[i] >> bit) & 1)
    //         {
    //             ESP_LOGI(TAG, "1");
    //         }
    //         else
    //         {
    //             ESP_LOGI(TAG, "0");
    //         }
    //     }

    // Turn on PDM output
    ESP_ERROR_CHECK(i2s_channel_enable(tx_channel));

    for (int i = 0; i < len; i++)
        for (int bit = 7; bit >= 0; bit--)
        {
            if ((data[i] >> bit) & 1)
            {
                for (int tot_bytes = 0; tot_bytes < duration_i2s; tot_bytes += w_bytes)
                {
                    /* Play the tone one */
                    if (i2s_channel_write(tx_channel, w_buf_one, duration_sine_one * sizeof(int16_t), &w_bytes, 1000) != ESP_OK)
                    {
                        printf("Write Task: i2s write failed\n");
                    }
                }
            }
            else
            {
                for (int tot_bytes = 0; tot_bytes < duration_i2s; tot_bytes += w_bytes)
                {
                    /* Play the tone zero */
                    if (i2s_channel_write(tx_channel, w_buf_zero, duration_sine_zero * sizeof(int16_t), &w_bytes, 1000) != ESP_OK)
                    {
                        printf("Write Task: i2s write failed\n");
                    }
                }
            }
        }

    // Turn off PDM output
    ESP_ERROR_CHECK(i2s_channel_disable(tx_channel));

    // Deallocate temp buffer
    free(w_buf_zero);
    free(w_buf_one);
}

void AUDIO_Init(void)
{
    // Initialize event group
    audioEventGroup = xEventGroupCreate();
}