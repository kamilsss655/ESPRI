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
#include <esp_err.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>

#include "sd.h"

static const char *TAG = "HW/SD";

sdmmc_card_t *card;

// Init SD card
esp_err_t SD_Init(void)
{
    ESP_LOGI(TAG, "Turning power on for the SD card");

    // Turn on P-MOSFET powering the SD card
    ESP_ERROR_CHECK(gpio_reset_pin(CONFIG_SD_CARD_ENABLE_GPIO));
    ESP_ERROR_CHECK(gpio_set_direction(CONFIG_SD_CARD_ENABLE_GPIO, GPIO_MODE_OUTPUT));
    gpio_set_level(CONFIG_SD_CARD_ENABLE_GPIO, 0);

    // Wait 50ms to ensure SD card is on
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // Options for mounting the filesystem.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4, // max number of open files
        .allocation_unit_size = 32 * 1024};

    ESP_LOGI(TAG, "Initializing SD card");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // Set frequency to 40Mhz (max), lower it if you run into issues
    // host.max_freq_khz = 40000;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Set 4-bit MMC bus width (fastest)
    slot_config.width = 4;

    // Uncomment if you run into issues:
    // gpio_reset_pin(2);
    // gpio_set_direction(2, GPIO_MODE_INPUT);
    // gpio_pullup_en(2);

    // Enable internal pullups
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(SD_BASE_PATH, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. ");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card is inserted.",
                     esp_err_to_name(ret));
        }
    }
    else
    {
        ESP_LOGI(TAG, "SD card successfully mounted at: %s", SD_BASE_PATH);

        // Card has been initialized, print its properties
        sdmmc_card_print_info(stdout, card);
    }
    return ret;
}

// Shutdown SD card
esp_err_t SD_Shutdown(void)
{
    // Unmount the SD card
    esp_vfs_fat_sdcard_unmount(SD_BASE_PATH, card);
    // Turn off P-MOSFET powering the SD card
    ESP_ERROR_CHECK(gpio_reset_pin(CONFIG_SD_CARD_ENABLE_GPIO));

    return ESP_OK;
}

// Format SD card
esp_err_t SD_Format(void)
{
        esp_err_t ret = esp_vfs_fat_sdcard_format(SD_BASE_PATH, card);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
        }
        else
        {
            ESP_LOGI(TAG, "SD card formatted");
        }

        return ret;
}