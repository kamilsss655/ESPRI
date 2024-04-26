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
#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_log.h>

#include "audio.h"
#include "settings.h"
#include "helper/misc.h"
#include "helper/rtos.h"
#include "web/handlers/websocket.h"

static const char *TAG = "HW/AUDIO";

static TaskHandle_t audioListenTaskHandle;

EventGroupHandle_t audioEventGroup;

adc_unit_t audioAdcUnit;

// ADC dropped frames count due to slow processing
volatile u_int16_t adcDroppedFrames = 0;

AudioState_t gAudioState;

SemaphoreHandle_t gAudioStateSemaphore;
SemaphoreHandle_t transmitSemaphore;

#define CALIBRATION_SAMPLES_TARGET 100

int calibrated = 0;
int16_t calibration_val = 0;
int calibration_finished = 0;

// There is an issue since this value will change with voltage, temperature etc
// for now let's test it
void AUDIO_AdcCalibrate(u_int16_t value)
{
    if (calibration_finished == 1)
        return;

    ESP_LOGI(TAG, "ADC value: %d", value);
    if (calibrated <= CALIBRATION_SAMPLES_TARGET)
    {
        calibrated += 1;
        calibration_val += value;

        if (calibrated > 2)
        {
            calibration_val /= 2;
        }
    }
    else
    {
        calibration_finished = 1;
        ESP_LOGI(TAG, "Mean ADC value: %d", calibration_val);
    }
}

// I2S handle to receive/listen audio
static adc_continuous_handle_t adc_handle;

const char *gAudioStateNames[4] = {
    "OFF",
    "LISTENING",
    "RECEIVING",
    "TRANSMITTING"};

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

// Consider this to be state machine?
// so we can have busy channel lock etc?
// Set gAudioState, protect with semaphores to prevent race conditions
static void AUDIO_SetAudioState(AudioState_t state)
{
    if (xSemaphoreTake(gAudioStateSemaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE)
    {
        gAudioState = state;

        WEBSOCKET_Send("gAudioState", "%s", gAudioStateNames[state]);

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
        .sample_freq_hz = AUDIO_INPUT_SAMPLE_FREQ,
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

    ESP_LOGI(TAG, "ADC initialized.");
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

void AUDIO_MonitorInput(void *pvParameters)
{
    uint16_t lastAudioCount = audioOnCount;
    while (1)
    {
        ESP_LOGE(TAG, "sound count: %d", audioOnCount);
        ESP_LOGI(TAG, "ADC dropped frames: %d", adcDroppedFrames);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (lastAudioCount != audioOnCount)
        {
            ESP_LOGI(TAG, "RECEIVING");
            if (gAudioState != AUDIO_RECEIVING)
                AUDIO_SetAudioState(AUDIO_RECEIVING);
        }
        else
        {
            ESP_LOGI(TAG, "LISTENING");
            if (gAudioState != AUDIO_LISTENING)
                AUDIO_SetAudioState(AUDIO_LISTENING);
        }
        lastAudioCount = audioOnCount;
    }
}

// Task listening to incoming audio on ADC port
void AUDIO_Listen(void *pvParameters)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[AUDIO_INPUT_CHUNK_SIZE] = {0};
    memset(result, 0xcc, AUDIO_INPUT_CHUNK_SIZE);
    audioListenTaskHandle = xTaskGetCurrentTaskHandle();
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
            AUDIO_SetAudioState(AUDIO_LISTENING);
            // report dropped frames for diagnostics
            ESP_LOGI(TAG, "ADC dropped frames: %d", adcDroppedFrames);
        }
        else // if no bits are set do listen (default state)
        {
            // Block until we receive notification from the interupt that data is available
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ret = adc_continuous_read(adc_handle, result, AUDIO_INPUT_CHUNK_SIZE, &ret_num, 0);

            if (ret == ESP_OK)
            {
                // ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
                {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t *)&result[i];
                    uint32_t chan_num = AUDIO_ADC_GET_CHANNEL(p);
                    uint16_t data = AUDIO_ADC_GET_DATA(p);

                    /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                    if (chan_num < SOC_ADC_CHANNEL_NUM(audioAdcUnit))
                    {
                        AUDIO_AdcCalibrate(data);

                        if ((data > ((calibration_val * 102) / 100)) || (data < ((calibration_val * 98) / 100)))
                        {
                            audioOnCount++;
                            // ESP_LOGI(TAG, "audio detected");
                        }
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Invalid data");
                    }
                }
                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */
                // vTaskDelay(1);
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
    // Initialize event group
    audioEventGroup = xEventGroupCreate();

    gAudioStateSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gAudioStateSemaphore);

    transmitSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(transmitSemaphore);

    initialize_pwm_audio();
}