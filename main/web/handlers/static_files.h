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

#ifndef STATIC_FILES_H
#define STATIC_FILES_H

#include <esp_err.h>
#include <esp_http_server.h>

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

esp_err_t STATIC_FILES_Download(httpd_req_t *req);
esp_err_t STATIC_FILES_Upload(httpd_req_t *req);
const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize);
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename);

#endif