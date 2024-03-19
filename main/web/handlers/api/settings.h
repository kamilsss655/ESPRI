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

#ifndef API_SETTINGS_H
#define API_SETTINGS_H

#include <esp_err.h>
#include <esp_http_server.h>

typedef struct
{
    char  *attr;     // json attr representing given value 
    void  *val;      // pointer to value
    bool  isInteger; // determines whether value is integer or string
} ApiSetting_t;

esp_err_t API_SETTINGS_Index(httpd_req_t *req);
esp_err_t API_SETTINGS_Create(httpd_req_t *req);

#endif
