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
#include <unistd.h>
#include <sys/stat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <pwm_audio.h>
#include <esp_adc/adc_continuous.h>
#include <esp_spiffs.h>
#include <freertos/ringbuf.h>
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "audio.h"
#include "settings.h"
#include "helper/misc.h"
#include "helper/rtos.h"
#include "hardware/led.h"
#include "hardware/sd.h"
#include "web/handlers/websocket.h"
#include "helper/filesystem.h"
#include <dsp/agc.h>
#ifdef AUDIO_RECORDER_FILTER_ENABLED
#include "dsp/filter.h"
#endif

static const char *TAG = "HW/AUDIO";

static TaskHandle_t audioListenTaskHandle;

EventGroupHandle_t audioEventGroup;

// ADC ring buffer handle
RingbufHandle_t adcRingBufferHandle;

adc_unit_t audioAdcUnit;
// ADC dropped frames count due to slow processing
volatile u_int16_t adcDroppedFrames = 0;

// Counts samples over squelch
uint16_t samplesOverSquelch;

AudioState_t gAudioState;

SemaphoreHandle_t gAudioStateSemaphore;
// Guards audio output shared resource
SemaphoreHandle_t transmitSemaphore;
// Guards audio input shared resource
SemaphoreHandle_t receiveSemaphore;
// Auto Gain Control handle
AGC_t agc;

// Apply settings like sample rate, volume, etc
static void pwm_audio_apply_settings(void)
{
    pwm_audio_set_param(AUDIO_OUTPUT_SAMPLE_FREQ, AUDIO_OUTPUT_BITS_PER_SAMPLE, 1);

    int16_t volume = (32 * gSettings.audio.out.volume) / 100;
    int8_t adjusted_volume = -16 + volume;
    // Set volume -16 to 16 based on settings
    pwm_audio_set_volume(adjusted_volume);
}

// Calibrate ADC by calculating mean value of the samples
void AUDIO_AdcCalibrate(void *pvParameters)
{
    // ADC sample
    AUDIO_ADC_DATA_TYPE *data;
    size_t received_data_size;
    uint32_t calibration_value = 0;

    // If ADC ring buffer is being used by some other task wait indefinitely
    while (xSemaphoreTake(receiveSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
    {
        ESP_LOGW(TAG, "Calibration waiting for ADC ring buffer to be released..");
    }

    for (u_int i = 0; i < AUDIO_ADC_CALIBRATION_SAMPLES; i++)
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
            ESP_LOGI(TAG, "Calibration failed. ADC ring buffer empty.");
            goto Done;
        }
    }

    gSettings.calibration.adc.value = (u_int16_t)calibration_value;
    gSettings.calibration.adc.is_valid = SETTINGS_TRUE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(SETTINGS_Save());

    ESP_LOGI(TAG, "ADC calibrated to: %d. Used %d samples for calibration.", gSettings.calibration.adc.value, AUDIO_ADC_CALIBRATION_SAMPLES);
Done:
    // Release semaphore so others can use the resource
    xSemaphoreGive(receiveSemaphore);
    // Delete self
    vTaskDelete(NULL);
}

// Low priority task that will consume ADC samples from the ADC ring buffer if no other tasks need them
// to prevent ADC ring buffer overflow
void AUDIO_EmptyAdcRingBuffer(void *pvParameters)
{
    // ADC sample
    AUDIO_ADC_DATA_TYPE *data;
    size_t received_data_size;

    while (1)
    {
        // Check if we can discard samples
        if (xSemaphoreTake(receiveSemaphore, 2000 / portTICK_PERIOD_MS) == pdFALSE)
        {
            ESP_LOGI(TAG, "ADC ring buffer is being accessed by some task.");
        }
        else
        {
            data = (AUDIO_ADC_DATA_TYPE *)xRingbufferReceive(adcRingBufferHandle, &received_data_size, pdMS_TO_TICKS(1000));
            // Check received data
            if (data != NULL)
            {
                // Return item so it gets removed from the ring buffer
                vRingbufferReturnItem(adcRingBufferHandle, (void *)data);
            }
            else
            {
                // Likely the buffer is empty
                ESP_LOGI(TAG, "ADC ring buffer empty");
            }

            // Give semaphore so other tasks can access ADC ring buffer
            xSemaphoreGive(receiveSemaphore);
        }
    }
}

// Audio record task
void AUDIO_Record(void *pvParameters)
{
#ifdef AUDIO_RECORDER_FILTER_ENABLED
    // Define filters
    FILTER_BiquadFilter_t hp_filter;
    FILTER_BiquadFilter_t lp_filter_1;
    FILTER_BiquadFilter_t lp_filter_2;
    FILTER_BiquadFilter_t lp_filter_3;
    // Init filters
    FILTER_Init(&hp_filter, AUDIO_INPUT_HPF_FREQ, AUDIO_INPUT_SAMPLE_FREQ, FILTER_HIGHPASS, 0.6);
    FILTER_Init(&lp_filter_1, AUDIO_INPUT_LPF_1_FREQ, AUDIO_INPUT_SAMPLE_FREQ, FILTER_LOWPASS, 0.25);
    FILTER_Init(&lp_filter_2, AUDIO_INPUT_LPF_2_FREQ, AUDIO_INPUT_SAMPLE_FREQ, FILTER_LOWPASS, 0.40);
    FILTER_Init(&lp_filter_3, AUDIO_INPUT_LPF_3_FREQ, AUDIO_INPUT_SAMPLE_FREQ, FILTER_LOWPASS, 0.40);
#endif

    // Retrieve params
    AUDIO_RecordParam_t *param = (AUDIO_RecordParam_t *)pvParameters;

    int16_t *buffersigned = NULL;

    FILE *fd = NULL;
    struct stat file_stat;

    // If the file exists delete it
    if (stat(param->filepath, &file_stat) == 0)
    {
        // Delete the file
        delete_file(param->filepath);
    }

    if (get_path_type(param->filepath) == FILESYSTEM_PATH_FLASH)
    {
        ESP_LOGI(TAG, "Performing garbage collection..");
        // Garbage collect to get enough free space for the file
        esp_err_t ret = esp_spiffs_gc(NULL, (param->duration_sec * AUDIO_INPUT_SAMPLE_FREQ * sizeof(AUDIO_ADC_DATA_TYPE)));
        // esp_err_t ret = esp_spiffs_gc(NULL, ((param->max_duration_ms / 1000) * AUDIO_INPUT_SAMPLE_FREQ * sizeof(AUDIO_ADC_DATA_TYPE)));
        if (ret != ESP_OK)
        {
            ESP_LOGI(TAG, "Recorder failed to allocate space");
            goto Done;
        }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Preparing recording.");

    ESP_LOGI(TAG, "Opening file: %s", param->filepath);
    fd = fopen(param->filepath, "wb");

    if (NULL == fd)
    {
        ESP_LOGE(TAG, "Failed to read %s", param->filepath);
        goto Done;
    }

    ESP_LOGI(TAG, "File opened");

    // Determines how many samples we want to save
    const size_t target_samples_written = param->duration_sec * AUDIO_INPUT_SAMPLE_FREQ;
    size_t bytes_received = 0;
    size_t samples_written = 0;

    // write wav header
    wav_header_t wav_header = {
        .ChunkID = "RIFF",
        .ChunkSize = (2 * target_samples_written) - 8,
        .Format = "WAVE",
        .Subchunk1ID = "fmt ",
        .Subchunk1Size = 16,
        .AudioFormat = 1,
        .NumChannels = 1,
        .SampleRate = AUDIO_INPUT_SAMPLE_FREQ,
        .ByteRate = 64000,
        .BlockAlign = 2,
        .BitsPerSample = 16,
        .Subchunk2ID = "data",
        .Subchunk2Size = target_samples_written * 2};

    int len = fwrite(&wav_header, 1, sizeof(wav_header_t), fd);

    if (len <= 0)
    {
        ESP_LOGE(TAG, "Read wav header failed");
    }

    ESP_LOGI(TAG, "Waiting for squelch to open");

    while (1)
    {
        // Wait until squelch opens
        while (gAudioState != AUDIO_RECEIVING)
        {
            vTaskDelay(1);
        }

        // If ADC ring buffer is being used by some other task wait indefinitely
        while (xSemaphoreTake(receiveSemaphore, 1000 / portTICK_PERIOD_MS) == pdFALSE)
        {
            ESP_LOGW(TAG, "Record waiting for ADC ring buffer to be released..");
        }

        // Wait until the buffer has enough data, to avoid writing to filesystem in small chunks
        while (xRingbufferGetCurFreeSize(adcRingBufferHandle) >= AUDIO_INPUT_CHUNK_SIZE)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        // Get ADC data from the ADC ring buffer
        buffersigned = xRingbufferReceiveUpTo(adcRingBufferHandle, &bytes_received, pdMS_TO_TICKS(250), AUDIO_INPUT_CHUNK_SIZE * sizeof(AUDIO_ADC_DATA_TYPE));

        ESP_LOGI(TAG, "bytes received: %d", bytes_received);

        // Check received data
        if (buffersigned != NULL)
        {
            // iterate over samples
            for (size_t i = 0; i < bytes_received / sizeof(AUDIO_ADC_DATA_TYPE); i += 1)
            {
                // Remove DC bias (center signal)
                buffersigned[i] = buffersigned[i] - gSettings.calibration.adc.value;
                // Amplify signal using AGC (clipping prevention built-in)
                buffersigned[i] = AGC_Update(&agc, buffersigned[i]);
#ifdef AUDIO_RECORDER_FILTER_ENABLED
                // Filter through high-pass filter
                buffersigned[i] = FILTER_Update(&hp_filter, buffersigned[i]);
                // Filter through 1st order low-pass filter
                buffersigned[i] = FILTER_Update(&lp_filter_1, buffersigned[i]);
                // Filter through 2nd order low-pass filter
                buffersigned[i] = FILTER_Update(&lp_filter_2, buffersigned[i]);
                // Amplify a bit between filtering to prevent filtering distortions
                buffersigned[i] *= 2;
                // Filter through 3rd order low-pass filter
                buffersigned[i] = FILTER_Update(&lp_filter_3, buffersigned[i]);
#endif
            }
            // Return item so it gets removed from the ring buffer
            vRingbufferReturnItem(adcRingBufferHandle, buffersigned);
            // Write to file
            samples_written += fwrite(buffersigned, 1, bytes_received, fd) / sizeof(AUDIO_ADC_DATA_TYPE);
            // Give semaphore so other tasks can access ADC ring buffer
            xSemaphoreGive(receiveSemaphore);

            // Check if we've written enough samples
            if (samples_written >= target_samples_written)
            {
                goto Done;
            }
        }
        else
        {
            // Likely the buffer is empty
            ESP_LOGI(TAG, "ADC ring buffer empty");
            goto Done;
        }
    }

Done:
    fclose(fd);

    ESP_LOGI(TAG, "Written recording to %s", param->filepath);

    // Delete self
    vTaskDelete(NULL);
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

// Monitors audio state for issues
void AUDIO_Watchdog(void *pvParameters)
{
    TaskHandle_t adcCalibrateTaskHandle = NULL;
    while (1)
    {
        // Schedule ADC calibration if its value is not valid
        if (gSettings.calibration.adc.is_valid == SETTINGS_FALSE)
        {
            ESP_LOGW(TAG, "ADC calibration is invalid. Please remove audio input. Scheduling calibration..");
            // If ADC calibrate task is not already running we can start
            if (adcCalibrateTaskHandle == NULL)
            {
                ESP_LOGI(TAG, "ADC calibration starting..");
                xTaskCreate(AUDIO_AdcCalibrate, "AUDIO_AdcCalibrate", 2048, NULL, RTOS_PRIORITY_HIGH, &adcCalibrateTaskHandle);
            }
        }
        // Check if there are dropped frames
        if (adcDroppedFrames > 0)
        {
            ESP_LOGW(TAG, "Dropped frames: %d", adcDroppedFrames);
            adcDroppedFrames = 0;
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
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
        // Squelch off delay based on the under the squelch time window count (200*10ms = 2000ms)
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
                        UBaseType_t res = xRingbufferSend(adcRingBufferHandle, &data, sizeof(AUDIO_ADC_DATA_TYPE), pdMS_TO_TICKS(1000));

                        if (res != pdTRUE)
                        {
                            ESP_LOGE(TAG, "Failed to send ADC data to the ring buffer. The buffer is full?");
                        }
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Invalid ADC data");
                    }
                }
                // Feed the watchdog
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
    int16_t *w_buf = (int16_t *)calloc(1, AUDIO_OUTPUT_BUFFER_SIZE);
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

    pwm_audio_apply_settings();

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
    int16_t *w_buf_one = (int16_t *)calloc(1, AUDIO_OUTPUT_BUFFER_SIZE);
    assert(w_buf_one);

    // Allocate temp buffer zero
    int16_t *w_buf_zero = (int16_t *)calloc(1, AUDIO_OUTPUT_BUFFER_SIZE);
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

    pwm_audio_apply_settings();

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

    pwm_audio_apply_settings();

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
        .ringbuf_len = AUDIO_OUTPUT_RING_BUFFER_SIZE,
    };
    pwm_audio_init(&pac);

    ESP_LOGI(TAG, "Audio output initialized on pin: %d", gSettings.gpio.audio_out);
}

void AUDIO_Init(void)
{
    // Initialize event group
    audioEventGroup = xEventGroupCreate();

    // Initialize semaphores
    gAudioStateSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gAudioStateSemaphore);

    transmitSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(transmitSemaphore);

    receiveSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(receiveSemaphore);

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
    // Init AGC
    AGC_Init(&agc, AUDIO_INPUT_AGC_INITIAL_GAIN);
}