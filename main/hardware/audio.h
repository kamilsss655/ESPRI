#pragma once

#define EXAMPLE_BUFF_SIZE   2048

#define EXAMPLE_I2S_BCLK_IO1        GPIO_NUM_4      // I2S bit clock io number
#define EXAMPLE_I2S_WS_IO1          GPIO_NUM_5      // I2S word select io number
#define EXAMPLE_I2S_DOUT_IO1        GPIO_NUM_18     // I2S data out io number

#define EXAMPLE_I2S_BCLK_IO2        GPIO_NUM_22     // I2S bit clock io number
#define EXAMPLE_I2S_WS_IO2          GPIO_NUM_23     // I2S word select io number
#define EXAMPLE_I2S_DOUT_IO2        GPIO_NUM_25     // I2S data out io number
#define EXAMPLE_I2S_DIN_IO2         GPIO_NUM_26     // I2S data in io number

/**
 * @brief I2S PDM TX example task
 *
 * @param args  The user data given from task creating, not used in this example
 */
void i2s_example_pdm_tx_task(void *args);
void example_beeper(void *args);
void AUDIO_PlayTone(uint16_t freq, int16_t duration_ms);