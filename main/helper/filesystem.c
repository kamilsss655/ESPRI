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

#include <esp_err.h>
#include <esp_spiffs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "filesystem.h"
#include "board.h"
#include <hardware/sd.h>

// Delete file and garbage collect to prevent issues
esp_err_t delete_file(const char *filepath)
{
    struct stat file_stat;

    // If file exists
    if (stat(filepath, &file_stat) == 0)
    {
        // Delete the file
        unlink(filepath);

        if (get_path_type(filepath) == FILESYSTEM_PATH_FLASH)
        {
            // Garbage collect to get enough free space for the file
            esp_err_t ret = esp_spiffs_gc(NULL, file_stat.st_size);
            return ret;
        }
        return ESP_OK;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}

// returns path type based on filepath
FILESYSTEM_Path_t get_path_type(const char *filepath)
{
    // if filepath starts with /storage
    if (strncmp(filepath, FLASH_BASE_PATH, strlen(FLASH_BASE_PATH)) == 0)
    {
        return FILESYSTEM_PATH_FLASH;
    }
    // if filepath starts with /sd
    else if (strncmp(filepath, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
    {
        return FILESYSTEM_PATH_SD;
    }
    else
    {
        return FILESYSTEM_PATH_UNKNOWN;
    }
}
