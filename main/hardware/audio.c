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
#include <sys/stat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <pwm_audio.h>
#include <esp_adc/adc_continuous.h>
#include <freertos/ringbuf.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "audio.h"
#include "settings.h"
#include "helper/misc.h"
#include "helper/rtos.h"
#include "hardware/led.h"
#include "web/handlers/websocket.h"

static const char *TAG = "HW/AUDIO";

static TaskHandle_t audioListenTaskHandle;

EventGroupHandle_t audioEventGroup;

// ADC ring buffer handle
RingbufHandle_t adcRingBufferHandle;

adc_unit_t audioAdcUnit;
bool written = true;
FILE *fd = NULL;
// ADC dropped frames count due to slow processing
volatile u_int16_t adcDroppedFrames = 0;

// Counts samples over squelch
uint16_t samplesOverSquelch;

AudioState_t gAudioState;

SemaphoreHandle_t gAudioStateSemaphore;
SemaphoreHandle_t transmitSemaphore;

/// @brief Calibrate ADC by calculating mean value of the samples
/// @param samples_count target samples count used for calibration
/// @return
esp_err_t AUDIO_AdcCalibrate(uint16_t samples_count)
{
    // ADC sample
    AUDIO_ADC_DATA_TYPE *data;
    size_t received_data_size;
    uint32_t calibration_value = 0;

    for (u_int i = 0; i < samples_count; i++)
    {
        // Get ADC data from the ADC ring buffer
        data = (AUDIO_ADC_DATA_TYPE *)xRingbufferReceive(adcRingBufferHandle, &received_data_size, pdMS_TO_TICKS(1000));
        // Check received data
        if (data != NULL)
        {
            // ESP_LOGI(TAG, "VAL:%d", *data);
            // Calculate mean value
            calibration_value += *data;
            if (i > 0)
            {
                calibration_value /= 2;
            }

            // Return item so it gets removed from the ring buffer
            vRingbufferReturnItem(adcRingBufferHandle, (void *)data);
        }
        else
        {
            // Likely the buffer is empty
            ESP_LOGI(TAG, "ADC ring buffer empty");
            return ESP_FAIL;
        }
    }

    gSettings.calibration.adc.value = (u_int16_t)calibration_value;
    gSettings.calibration.adc.is_valid = SETTINGS_TRUE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(SETTINGS_Save());

    ESP_LOGI(TAG, "ADC calibrated to: %d. Used %d samples for calibration.", gSettings.calibration.adc.value, samples_count);

    return ESP_OK;
}

esp_err_t AUDIO_Record(size_t samples_count)
{
    ESP_LOGI(TAG, "Starting recording.");
    // ADC sample
    size_t bytes_received = 0;
    size_t samples_written = 0;
    AUDIO_ADC_DATA_TYPE *data;
    const size_t chunk_size = AUDIO_INPUT_CHUNK_SIZE;
    AUDIO_ADC_DATA_TYPE *buffer = calloc(chunk_size, sizeof(AUDIO_ADC_DATA_TYPE));
    int16_t *buffersigned = calloc(chunk_size, sizeof(int16_t));

    // write wav header
    wav_header_t wav_header = {
        .Subchunk1ID = "fmt",
        .Subchunk2ID = "data",
        .SampleRate = AUDIO_INPUT_SAMPLE_FREQ,
        .NumChannels = 1,
        .BitsPerSample = 16};

    int len = fwrite(&wav_header, 1, sizeof(wav_header_t), fd);
    if (len <= 0)
    {
        ESP_LOGE(TAG, "Read wav header failed");
        fclose(fd);
        return ESP_FAIL;
    }

    for (u_int i = 0; i < samples_count; i += samples_written)
    {
        // Get ADC data from the ADC ring buffer
        // bytes received is indeed bytes, so now we receive 400 bytes (chunk 200 * sizeof 2 = 400), so thats 200 samples
        buffer = (AUDIO_ADC_DATA_TYPE *)xRingbufferReceiveUpTo(adcRingBufferHandle, &bytes_received, pdMS_TO_TICKS(250), chunk_size * sizeof(AUDIO_ADC_DATA_TYPE));
        // Check received data
        if (buffer != NULL)
        {
            // ESP_LOGI(TAG, "bytes_received: %zu", bytes_received);
            // iterate over samples
            for (size_t i = 0; i < bytes_received / sizeof(AUDIO_ADC_DATA_TYPE); i += 1)
            {
                // this is 1.5 upsample kind off, so need to listen at 24kHz
                // if (i == (bytes_received / 2) - 1)
                // {
                    // last sample in buffer might be not present if we received odd number of samples, so we just use single sample to avoid glitching
                buffersigned[i] = buffer[i];
                // }
                // else
                // {
                //     buffersigned[i] = (buffer[i] + buffer[i + 1]) / 2;
                // }

                // Remove DC bias (center signal)
                buffersigned[i] = buffersigned[i] - gSettings.calibration.adc.value;
                // Amplify
                buffersigned[i] *= 20;
            }
            // Return item so it gets removed from the ring buffer
            vRingbufferReturnItem(adcRingBufferHandle, (void *)buffer);
            // write to file
            samples_written = fwrite(buffersigned, sizeof(int16_t), bytes_received / sizeof(AUDIO_ADC_DATA_TYPE), fd);
        }
        else
        {
            // Likely the buffer is empty
            ESP_LOGI(TAG, "ADC ring buffer empty");
            return ESP_FAIL;
        }
    }

    fclose(fd);

    // free(buffer);
    // free(buffersigned);

    written = true;

    ESP_LOGI(TAG, "Written recording to sample.wav");

    return ESP_OK;
}

// I2S handle to receive/listen audio
static adc_continuous_handle_t adc_handle;

// Interupt callback called when ADC finishes one portion of ADC conversions
static bool IRAM_ATTR adc_conv_done_callback(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    // Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(audioListenTaskHandle, &mustYield);

    return (mustYield == pdTRUE);
}

// Interupt callback called when there is ADC sample overflow likely due to slow data processing
static bool IRAM_ATTR adc_pool_ovf(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    adcDroppedFrames++;
    return false;
}

// Set gAudioState
static void AUDIO_SetAudioState(AudioState_t state)
{
    if (xSemaphoreTake(gAudioStateSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE)
    {
        gAudioState = state;

        // Side effects
        switch (gAudioState)
        {
        case AUDIO_TRANSMITTING:
        case AUDIO_RECEIVING:
            LED_Fade(LED_BRIGHTNESS_MAX, LED_TIME_FAST, false);
            // Take LED semaphore to prevent other tasks interacting with the LED
            xSemaphoreTake(gLedSemaphore, LED_TIME_MAX / portTICK_PERIOD_MS);
            break;

        default:
            // Give LED semaphore to allow other tasks interacting with the LED
            xSemaphoreGive(gLedSemaphore);
            LED_Fade(LED_BRIGHTNESS_OFF, LED_TIME_FAST, false);
            break;
        }

        WEBSOCKET_Send("gAudioState", "%d", state);

        xSemaphoreGive(gAudioStateSemaphore);
    }
    else
    {
        ESP_LOGW(TAG, "Could not update gAudioState due to lock.");
    }
}
// Initialize ADC
static void AUDIO_AdcInit()
{
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = AUDIO_INPUT_MAX_BUFF_SIZE,
        .conv_frame_size = AUDIO_INPUT_CHUNK_SIZE,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = (AUDIO_INPUT_SAMPLE_FREQ * AUDIO_INPUT_UPSAMPLE_FACTOR * AUDIO_INPUT_SAMPLE_RATE_WORKAROUND),
        .conv_mode = AUDIO_ADC_CONV_MODE,
        .format = AUDIO_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};

    static adc_channel_t audioAdcChannel;

    // set adc channel and unit based on audio input GPIO number
    adc_continuous_io_to_channel(gSettings.gpio.audio_in, &audioAdcUnit, &audioAdcChannel);

    dig_cfg.pattern_num = 1;

    adc_pattern[0].atten = AUDIO_ADC_ATTEN;
    adc_pattern[0].atten = AUDIO_ADC_ATTEN;
    adc_pattern[0].channel = audioAdcChannel;
    adc_pattern[0].unit = audioAdcUnit;
    adc_pattern[0].bit_width = AUDIO_ADC_BIT_WIDTH;

    dig_cfg.adc_pattern = adc_pattern;

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = adc_conv_done_callback,
        .on_pool_ovf = adc_pool_ovf};
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

    ESP_LOGI(TAG, "ADC initialized");
    ESP_LOGI(TAG, "ADC attenuation: %" PRIx8, dig_cfg.adc_pattern[0].atten);
    ESP_LOGI(TAG, "ADC channel: %" PRIx8, dig_cfg.adc_pattern[0].channel);
}

// Stop ADC
void AUDIO_AdcStop()
{
    adc_continuous_stop(adc_handle);
    adc_continuous_deinit(adc_handle);
}

u_int16_t audioOnCount = 0;

// Main audio processing task that reads ADC samples from the ADC ring buffer and processes them
void AUDIO_AudioInputProcess(void *pvParameters)
{
    // ADC sample
    AUDIO_ADC_DATA_TYPE *data;
    size_t received_data_size;

    while (1)
    {
        // Recalibrate if current ADC calibration is invalid
        if (gSettings.calibration.adc.is_valid == SETTINGS_FALSE)
        {
            AUDIO_AdcCalibrate(AUDIO_ADC_CALIBRATION_SAMPLES);
        }
        // if squelch on record
        else if (written == false && gAudioState == AUDIO_RECEIVING)
        {
            AUDIO_Record((AUDIO_INPUT_SAMPLE_FREQ) * 7);
        }
        // otherwise just consume samples to prevent buffer overflow
        else
        {
            // Get ADC data from the ADC ring buffer
            // To get the value you need to dereference the pointer, so use *data
            data = (AUDIO_ADC_DATA_TYPE *)xRingbufferReceive(adcRingBufferHandle, &received_data_size, pdMS_TO_TICKS(1000));
            // Check received data
            if (data != NULL)
            {
                // Process data here

                // Return item so it gets removed from the ring buffer
                vRingbufferReturnItem(adcRingBufferHandle, (void *)data);
            }
            else
            {
                // Likely the buffer is empty
                ESP_LOGI(TAG, "ADC ring buffer empty");
            }
        }
    }
}

// Monitors audio state for issues
void AUDIO_Watchdog(void *pvParameters)
{
    while (1)
    {
        if (adcDroppedFrames > 0)
        {
            ESP_LOGW(TAG, "Dropped frames: %d", adcDroppedFrames);
            adcDroppedFrames = 0;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
// Monitors samples over squelch and controls the squelch
void AUDIO_SquelchControl(void *pvParameters)
{
    // Last seen samples over squelch, updated every time window
    uint16_t last_seen_samples_over_squelch = samplesOverSquelch;
    // Counts time windows when there were samples over squelch
    u_int16_t over_squelch_windows = 0;
    // Counts time windows when there where samples under squelch
    u_int16_t under_squelch_windows = 0;
    while (1)
    {
        // If the samplesOverSquelch has changed since last time we checked it means we are receiving audio
        if (last_seen_samples_over_squelch != samplesOverSquelch)
        {
            // Reset the opposite window
            under_squelch_windows = 0;
            // Prevent overflow
            samplesOverSquelch = 0;
            if (gAudioState == AUDIO_LISTENING)
                over_squelch_windows++;
        }
        // If the samplesOverSquelch remains unchanged since last time we checked it means we are not receiving audio
        else if (last_seen_samples_over_squelch == samplesOverSquelch)
        {
            // Reset the opposite window
            over_squelch_windows = 0;
            if (gAudioState == AUDIO_RECEIVING)
                under_squelch_windows++;
        }

        // Squelch on delay based on the over the squelch time window count (1*10ms = 10ms)
        if (over_squelch_windows >= 1 && gAudioState != AUDIO_RECEIVING)
        {
            ESP_LOGI(TAG, "RECEIVING");
            AUDIO_SetAudioState(AUDIO_RECEIVING);
            over_squelch_windows = 0;
        }
        // Squelch off delay based on the under the squelch time window count (10*10ms = 100ms)
        if (under_squelch_windows >= 200 && (gAudioState != AUDIO_LISTENING && gAudioState != AUDIO_TRANSMITTING))
        {
            ESP_LOGI(TAG, "LISTENING");
            AUDIO_SetAudioState(AUDIO_LISTENING);
            under_squelch_windows = 0;
        }

        // ESP_LOGI(TAG, "O:%d U:%d S:%d", over_squelch_windows, under_squelch_windows, gAudioState);
        // ESP_LOGI(TAG, "last:%d new:%d", last_seen_samples_over_squelch, samplesOverSquelch);

        // Set value for next time window
        last_seen_samples_over_squelch = samplesOverSquelch;

        // Delay of this task is the time multplier of the time windows (10ms)
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Task listening to incoming audio on ADC port
// It writes ADC samples to ADC ring buffer for further processing
void AUDIO_Listen(void *pvParameters)
{
    // ADC sample value
    AUDIO_ADC_DATA_TYPE data;
    adc_digi_output_data_t *p;
    uint32_t chan_num;
    uint32_t received_bytes = 0;
    esp_err_t ret;
    uint8_t result[AUDIO_INPUT_CHUNK_SIZE] = {0};
    memset(result, 0xcc, AUDIO_INPUT_CHUNK_SIZE);

    audioListenTaskHandle = xTaskGetCurrentTaskHandle();
    EventBits_t audioEventGroupBits;

    AUDIO_AdcInit();

    AUDIO_SetAudioState(AUDIO_LISTENING);

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
            AUDIO_SetAudioState(AUDIO_LISTENING);
        }
        else // if no bits are set do listen (default state)
        {
            // Block until we receive notification from the interupt that data frame is available
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ret = adc_continuous_read(adc_handle, result, AUDIO_INPUT_CHUNK_SIZE, &received_bytes, 0);

            if (ret == ESP_OK)
            {
                for (int i = 0; i < received_bytes; i += SOC_ADC_DIGI_RESULT_BYTES * AUDIO_INPUT_UPSAMPLE_FACTOR)
                {
                    // Calculate mean value from AUDIO_INPUT_UPSAMPLE_FACTOR samples
                    data = 0;
                    for (int up = 0; up < SOC_ADC_DIGI_RESULT_BYTES * AUDIO_INPUT_UPSAMPLE_FACTOR; up += SOC_ADC_DIGI_RESULT_BYTES)
                    {
                        p = (adc_digi_output_data_t *)&result[i + up];
                        chan_num = AUDIO_ADC_GET_CHANNEL(p);
                        // Get actual ADC sample value
                        data += AUDIO_ADC_GET_DATA(p);

                        if (up > 0)
                        {
                            data /= 2;
                        }
                    }

                    // Check the channel number validation, the data is invalid if the channel num exceed the maximum channel
                    if (chan_num < SOC_ADC_CHANNEL_NUM(audioAdcUnit))
                    {
                        // Count samples over the squelch threshold
                        if ((data > ((gSettings.calibration.adc.value * (100 + gSettings.audio.in.squelch)) / 100)) || (data < ((gSettings.calibration.adc.value * (100 - gSettings.audio.in.squelch)) / 100)))
                        {
                            samplesOverSquelch++;
                        }

                        // Send ADC sample to ring buffer
                        UBaseType_t res = xRingbufferSend(adcRingBufferHandle, &data, sizeof(AUDIO_ADC_DATA_TYPE), pdMS_TO_TICKS(100));

                        if (res != pdTRUE)
                        {
                            ESP_LOGW(TAG, "Failed to send ADC data to the ring buffer. The buffer is full?");
                        }
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Invalid ADC data");
                    }
                }
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
// This will stop audio listening so we can transmit
esp_err_t AUDIO_TransmitStart(void)
{
    EventBits_t audioEventGroupBits;

    // Check if we can re-schedule new transmission
    if (xSemaphoreTake(transmitSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
    {
        // Transmission in progress
        ESP_LOGW(TAG, "Overlapping audio output transmissions detected.");
        WEBSOCKET_Send(TAG, "Overlapping audio output transmissions detected.");
        return ESP_FAIL;
    }

    // request that we stop listening
    xEventGroupSetBits(audioEventGroup, BIT_STOP_LISTEN);

    // block until listening stopped
    do
    {
        audioEventGroupBits = xEventGroupGetBits(audioEventGroup);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while ((audioEventGroupBits & BIT_STOPPED_LISTENING) != BIT_STOPPED_LISTENING);

    AUDIO_SetAudioState(AUDIO_TRANSMITTING);

    ESP_LOGI(TAG, "Transmit starting");

    return ESP_OK;
}

// Stop audio transmit
// Releases event bits so audio listening can resume
esp_err_t AUDIO_TransmitStop(void)
{
    ESP_LOGI(TAG, "Transmit stop");

    // Indicate that audio transmit is finished
    xEventGroupClearBits(audioEventGroup, BIT_STOPPED_LISTENING);
    xEventGroupSetBits(audioEventGroup, BIT_DONE_TX);

    // Indicate that we are done and can transmit again
    xSemaphoreGive(transmitSemaphore);

    return ESP_OK;
}

/// @brief Play single tone
/// @param freq frequency of the tone in hz
/// @param duration_ms duration in ms
void AUDIO_PlayTone(uint16_t freq, uint16_t duration_ms)
{
    size_t w_bytes = 0;

    // Allocate temp buffer
    int16_t *w_buf = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf);

    uint32_t duration_sine = (AUDIO_OUTPUT_SAMPLE_FREQ / (float)freq) + 0.5;

    // Generate tone buffer for single sinewave
    for (int i = 0; i < duration_sine; i++)
    {
        w_buf[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine)) * (gSettings.audio.out.volume * AUDIO_VOLUME_MULTIPLIER));
    }

    // Multiply single sinewave to desired duration
    uint32_t duration_total = duration_ms * AUDIO_OUTPUT_SAMPLE_FREQ / 1000;

    // Multiply duration by 2 because we point to u_int8_t and write int16_t
    duration_total *= 2;

    // Turn on PWM audio output
    pwm_audio_set_param(AUDIO_OUTPUT_SAMPLE_FREQ, AUDIO_OUTPUT_BITS_PER_SAMPLE, 1U);
    pwm_audio_start();

    for (int tot_bytes = 0; tot_bytes < duration_total; tot_bytes += w_bytes)
    {
        // Play the tone
        if (pwm_audio_write((u_int8_t *)w_buf, duration_sine * sizeof(int16_t), &w_bytes, 1000 / portTICK_PERIOD_MS) != ESP_OK)
        {
            printf("Write Task: i2s write failed\n");
        }
    }

    // Stop audio
    pwm_audio_stop();

    // Deallocate temp buffer
    free(w_buf);
}

// Play AFSK coded data
void AUDIO_PlayAFSK(const uint8_t *data, size_t len, uint16_t baud, uint16_t zero_freq, uint16_t one_freq)
{
    uint16_t zero_freq_p;
    uint16_t one_freq_p;
    uint16_t baud_p;

    // Sanitize inputs
    zero_freq_p = MAX(zero_freq, AUDIO_AFSK_TONE_MIN_FREQ);
    one_freq_p = MAX(one_freq, AUDIO_AFSK_TONE_MIN_FREQ);
    baud_p = MAX(baud, AUDIO_AFSK_MIN_BAUD);

    zero_freq_p = MIN(zero_freq, AUDIO_AFSK_TONE_MAX_FREQ);
    one_freq_p = MIN(one_freq, AUDIO_AFSK_TONE_MAX_FREQ);
    baud_p = MIN(baud, AUDIO_AFSK_MAX_BAUD);

    int duration_us = (1000001 / baud_p);

    ESP_LOGI(TAG, "AFSK baud: %d, duration_us: %d", baud_p, duration_us);
    ESP_LOGI(TAG, "AFSK zero_f: %d, one_f: %d", zero_freq_p, one_freq_p);

    size_t w_bytes = 0;

    // Allocate temp buffer one
    int16_t *w_buf_one = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf_one);

    // Allocate temp buffer zero
    int16_t *w_buf_zero = (int16_t *)calloc(1, AUDIO_BUFFER_SIZE);
    assert(w_buf_zero);

    uint32_t duration_sine_zero = (AUDIO_OUTPUT_SAMPLE_FREQ / (float)zero_freq_p) + 0.5;
    uint32_t duration_sine_one = (AUDIO_OUTPUT_SAMPLE_FREQ / (float)one_freq_p) + 0.5;

    /* Generate the tone buffer */
    // Single sinewave zero

    for (int i = 0; i < duration_sine_zero; i++)
    {
        w_buf_zero[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine_zero)) * (gSettings.audio.out.volume * AUDIO_VOLUME_MULTIPLIER));
    }

    /* Generate the tone buffer */
    // Single sinewave one

    for (int i = 0; i < duration_sine_one; i++)
    {
        w_buf_one[i] = (int16_t)((sin(2 * (float)i * CONST_PI / duration_sine_one)) * (gSettings.audio.out.volume * AUDIO_VOLUME_MULTIPLIER));
    }

    // Multiply single sinewave to desired duration

    uint32_t duration_total = duration_us * AUDIO_OUTPUT_SAMPLE_FREQ / 1000000;

    // Multiply duration by 2 because we point to u_int8_t and write int16_t
    duration_total *= 2;

    ESP_LOGI(TAG, "AFSK duration_total: %" PRIu32 "", duration_total);

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

    // Turn on PWM audio output
    pwm_audio_set_param(AUDIO_OUTPUT_SAMPLE_FREQ, AUDIO_OUTPUT_BITS_PER_SAMPLE, 1U);
    pwm_audio_start();

    for (int i = 0; i < len; i++)
        for (int bit = 7; bit >= 0; bit--)
        {
            if ((data[i] >> bit) & 1)
            {

                for (int tot_bytes = 0; tot_bytes < duration_total; tot_bytes += w_bytes)
                {
                    // Play the tone one
                    if (pwm_audio_write((u_int8_t *)w_buf_one, duration_sine_one * sizeof(int16_t), &w_bytes, 1000 / portTICK_PERIOD_MS) != ESP_OK)
                    {
                        printf("Write Task: i2s write failed\n");
                    }
                }
            }
            else
            {
                for (int tot_bytes = 0; tot_bytes < duration_total; tot_bytes += w_bytes)
                {
                    // Play the tone zero
                    if (pwm_audio_write((u_int8_t *)w_buf_zero, duration_sine_zero * sizeof(int16_t), &w_bytes, 1000 / portTICK_PERIOD_MS) != ESP_OK)
                    {
                        printf("Write Task: i2s write failed\n");
                    }
                }
            }
        }

    // Stop audio
    pwm_audio_stop();

    // Deallocate temp buffer
    free(w_buf_zero);
    free(w_buf_one);
}

esp_err_t AUDIO_PlayWav(const char *filepath)
{
    FILE *fd = NULL;
    struct stat file_stat;

    if (stat(filepath, &file_stat) == -1)
    {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "file stat info: %s (%ld bytes)...", filepath, file_stat.st_size);
    fd = fopen(filepath, "r");

    if (NULL == fd)
    {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        return ESP_FAIL;
    }
    const size_t chunk_size = 4096;
    uint8_t *buffer = malloc(chunk_size);

    if (NULL == buffer)
    {
        ESP_LOGE(TAG, "audio data buffer malloc failed");
        fclose(fd);
        return ESP_FAIL;
    }

    /**
     * read head of WAV file
     */
    wav_header_t wav_head;
    int len = fread(&wav_head, 1, sizeof(wav_header_t), fd);
    if (len <= 0)
    {
        ESP_LOGE(TAG, "Read wav header failed");
        fclose(fd);
        return ESP_FAIL;
    }
    if (NULL == strstr((char *)wav_head.Subchunk1ID, "fmt") &&
        NULL == strstr((char *)wav_head.Subchunk2ID, "data"))
    {
        ESP_LOGE(TAG, "Header of wav format error");
        fclose(fd);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "frame_rate= %" PRIi32 ", ch=%d, width=%d", wav_head.SampleRate, wav_head.NumChannels, wav_head.BitsPerSample);

    pwm_audio_set_param(wav_head.SampleRate, wav_head.BitsPerSample, wav_head.NumChannels);
    pwm_audio_start();

    /**
     * read wave data of WAV file
     */
    size_t write_num = 0;
    size_t cnt;

    do
    {
        /* Read file in chunks into the scratch buffer */
        len = fread(buffer, 1, chunk_size, fd);
        if (len <= 0)
        {
            break;
        }
        pwm_audio_write(buffer, len, &cnt, 1000 / portTICK_PERIOD_MS);

        write_num += len;
    } while (1);

    // Stop audio
    pwm_audio_stop();
    // Close file
    fclose(fd);
    // Deallocate temp buffer
    free(buffer);

    ESP_LOGI(TAG, "File reading complete, total: %d bytes", write_num);
    return ESP_OK;
}

// Init PWM audio
static void initialize_pwm_audio(void)
{
    pwm_audio_config_t pac = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .gpio_num_left = gSettings.gpio.audio_out,
        .ledc_channel_left = LEDC_CHANNEL_1,
        .gpio_num_right = gSettings.gpio.audio_out,
        .ledc_channel_right = LEDC_CHANNEL_1,
        .ledc_timer_sel = LEDC_TIMER_1,
        .ringbuf_len = 1024 * 8,
    };
    pwm_audio_init(&pac);

    ESP_LOGI(TAG, "Audio output initialized on pin: %d", gSettings.gpio.audio_out);
}

void AUDIO_Init(void)
{
    ESP_LOGI(TAG, "Opening file..");
    fd = fopen("/storage/sample.wav", "wb");

    if (NULL == fd)
    {
        ESP_LOGE(TAG, "Failed to read sample.wav");
    }
    ESP_LOGI(TAG, "File opened");

    written = false;

    // Initialize event group
    audioEventGroup = xEventGroupCreate();

    gAudioStateSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gAudioStateSemaphore);

    transmitSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(transmitSemaphore);

    // Create ADC ring buffer to store ADC samples (FIFO)
    adcRingBufferHandle = xRingbufferCreate(AUDIO_ADC_RING_BUFFER_SIZE, AUDIO_ADC_RING_BUFFER_TYPE);
    if (adcRingBufferHandle == NULL)
    {
        ESP_LOGE(TAG, "Failed to create ADC ring buffer");
    }
    else
    {
        ESP_LOGI(TAG, "ADC ring buffer initialized");
    }

    initialize_pwm_audio();
}