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

#ifndef HELPER_HTTP_H
#define HELPER_HTTP_H

#include <esp_vfs.h>
#include "hardware/http_server.h"

typedef struct
{
    // Base path of file storage
    char base_path[ESP_VFS_PATH_MAX + 1];

    // Scratch buffer for temporary storage during file transfer
    char scratch[SCRATCH_BUFSIZE];
} file_server_data;

#endif