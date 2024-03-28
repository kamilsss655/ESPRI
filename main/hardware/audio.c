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

#include <driver/i2s.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <driver/i2s_pdm.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_adc/adc_continuous.h>

#include "audio.h"
#include "settings.h"
#include "helper/rtos.h"

i2s_pin_config_t pin_config;

#define EXAMPLE_ADC_UNIT ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit) #unit
#define EXAMPLE_ADC_UNIT_STR(unit) _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12
#define EXAMPLE_ADC_BIT_WIDTH SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data) ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data) ((p_data)->type2.data)
#endif
#define EXAMPLE_READ_LEN 256
static adc_channel_t channel[1] = {ADC_CHANNEL_7};

static TaskHandle_t s_task_handle;

static const char *TAG = "HW/AUDIO";

EventGroupHandle_t audioEventGroup;

typedef enum
{
    BIT_STOP_LISTEN = (1 << 0),
    BIT_STOPPED_LISTENING = (1 << 1),
    BIT_DONE_TX = (1 << 2)
} AudioEventBit_t;

AudioState_t gAudioState;

i2s_chan_handle_t tx_channel;
static adc_continuous_handle_t rx_channel;

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void AUDIO_AdcInit()
{
    // adc_continuous_handle_t adc_handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = EXAMPLE_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &rx_channel));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 32 * 1000,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = 1;
    for (int i = 0; i < 1; i++)
    {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%" PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%" PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%" PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(rx_channel, &dig_cfg));

    // *out_handle = adc_handle;

    // continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &rx_channel);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(rx_channel, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(rx_channel));
}

void AUDIO_AdcStop()
{
    adc_continuous_stop(rx_channel);
    adc_continuous_deinit(rx_channel);
}

void AUDIO_Monitor(void *pvParameters)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[EXAMPLE_READ_LEN] = {0};
    memset(result, 0xcc, EXAMPLE_READ_LEN);
    s_task_handle = xTaskGetCurrentTaskHandle();
    char unit[] = EXAMPLE_ADC_UNIT_STR(EXAMPLE_ADC_UNIT);
    EventBits_t audioEventGroupBits;

    AUDIO_AdcInit();

    while (1)
    {
        audioEventGroupBits = xEventGroupGetBits(audioEventGroup);
        /**
         * This is to show you the way to use the ADC continuous mode driver event callback.
         * This `ulTaskNotifyTake` will block when the data processing in the task is fast.
         * However in this example, the data processing (print) is slow, so you barely block here.
         *
         * Without using this event callback (to notify this task), you can still just call
         * `adc_continuous_read()` here in a loop, with/without a certain block timeout.
         */

        // Check if we can re-schedule new transmission

        if ((audioEventGroupBits & BIT_STOP_LISTEN) != 0)
        {
            AUDIO_AdcStop();
            ESP_LOGI(TAG, "Stopped ADC");
            xEventGroupClearBits(audioEventGroup, BIT_STOP_LISTEN);
            xEventGroupSetBits(audioEventGroup, BIT_STOPPED_LISTENING);
        }
        else if ((audioEventGroupBits & BIT_STOPPED_LISTENING) != 0)
        {
            ESP_LOGI(TAG, "Listening waiting for tx to end");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else if ((audioEventGroupBits & BIT_DONE_TX) != 0)
        {
            ESP_LOGI(TAG, "Listening detected tx end, re-initializing adc");
            // re-init adc
            AUDIO_AdcInit();
            xEventGroupClearBits(audioEventGroup, BIT_DONE_TX);
        }
        else // if no bits set we listen by default
        {
            // monitor here
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ret = adc_continuous_read(rx_channel, result, EXAMPLE_READ_LEN, &ret_num, 0);
            if (ret == ESP_OK)
            {
                // ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
                {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                    uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                    uint32_t data = EXAMPLE_ADC_GET_DATA(p);
                    /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                    if (chan_num < SOC_ADC_CHANNEL_NUM(EXAMPLE_ADC_UNIT))
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
                // We try to read `EXAMPLE_READ_LEN` until API returns timeout, which means there's no available data
                // break;
                vTaskDelay(10);
            }
        }
    }

    // AUDIO_AdcStop();
}

// Initialize audio transmit
// Note: Keep wires/traces from PDM output pin as short as possible to minimalize interference
i2s_chan_handle_t AUDIO_Transmit(void)
{
    EventBits_t audioEventGroupBits;

    // request that we stop listening
    xEventGroupSetBits(audioEventGroup, BIT_STOP_LISTEN);
    // uxBits & ( BIT_0 | BIT_4 ) ) == ( BIT_0 | BIT_4 )
    // block until listening stopped
    do
    {
        // if we haven't stopped listening we will block an wait here
        audioEventGroupBits = xEventGroupGetBits(audioEventGroup);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while ((audioEventGroupBits & BIT_STOPPED_LISTENING) != BIT_STOPPED_LISTENING);

    ESP_LOGI(TAG, "bits: %lu", audioEventGroupBits);

    ESP_LOGI(TAG, "Transmit starting");

    gAudioState = AUDIO_TRANSMITTING;

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
esp_err_t AUDIO_TransmitFinish(void)
{

    ESP_LOGI(TAG, "Transmit finish");

    gAudioState = AUDIO_LISTENING;

    // i2s_channel_disable(tx_channel); // this is already disabled by beep?
    /* If the handle is not needed any more, delete it to release the channel resources */
    i2s_del_channel(tx_channel);

    xEventGroupClearBits(audioEventGroup, BIT_STOPPED_LISTENING);
    xEventGroupSetBits(audioEventGroup, BIT_DONE_TX);

    EventBits_t audioEventGroupBits;

    audioEventGroupBits = xEventGroupGetBits(audioEventGroup);

    ESP_LOGI(TAG, "bits after transmit finish: %lu", audioEventGroupBits);

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

    // AUDIO_Transmit();

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

    // Free I2S0 controller
    // AUDIO_TransmitFinish();
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

    // AUDIO_Transmit();

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

    // AUDIO_TransmitFinish();
    // Deallocate temp buffer
    free(w_buf_zero);
    free(w_buf_one);
}

void AUDIO_Init(void)
{
    audioEventGroup = xEventGroupCreate();

    // gAudioState = AUDIO_LISTENING;

    // AUDIO_AdcInit();

    // ESP_LOGI(TAG, "Initialized adc");
}