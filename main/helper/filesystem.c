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

// Delete file and garbage collect to prevent issues
esp_err_t delete_file(const char *filepath)
{
    struct stat file_stat;

    // If file exists
    if (stat(filepath, &file_stat) == 0)
    {
        // Delete the file
        unlink(filepath);

        // Garbage collect to get enough free space for the file
        esp_err_t ret = esp_spiffs_gc(NULL, file_stat.st_size);

        return ret;
    }
    else
    {
        return ESP_ERR_NOT_FOUND;
    }
}
