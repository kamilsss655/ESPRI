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

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_spiffs.h>

static const char *TAG = "HW/SPIFFS";

// Init SPIFFS
esp_err_t SPIFFS_Init(const char *base_path)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = base_path,
        .partition_label = NULL,
        .max_files = 5, // This sets the maximum number of files that can be open at the same time
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;

    ret = esp_spiffs_info(NULL, &total, &used);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Partition: %s size: total: %d, used: %d", base_path, total, used);
    return ESP_OK;
}

// Check integrity of SPIFFS (takes a lot of time)
esp_err_t SPIFFS_IntegrityCheck(void)
{      
    esp_err_t ret = esp_spiffs_check(NULL);

    if (ret != ESP_OK)
    {
        if (ret == ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "Partition was not mounted");
        }
        else if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Partition corrupted");
        }
        return ret;
    }

    ESP_LOGE(TAG, "Partition integrity check positive");

    return ESP_OK;
}