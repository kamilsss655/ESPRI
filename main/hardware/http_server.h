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

#ifndef HARDWARE_HTTP_SERVER_H
#define HARDWARE_HTTP_SERVER_H

#include <esp_http_server.h>
#include <esp_err.h>

/* Scratch buffer size */
#define SCRATCH_BUFSIZE 8192
#define HTTP_SERVER_MAX_URI_HANDLERS 20

extern httpd_handle_t gHttpServerHandle;

esp_err_t HTTP_SERVER_Init(const char *base_path);

#endif