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
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <esp_spiffs.h>
#include <esp_http_server.h>

#include "hardware/uart.h"

static const char *TAG = "WEB/API/EVENT_HANDLER";

// Redirect / requests to /index.html file
esp_err_t api_action_post_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_send(req, NULL, 0); // Response body can be empty
    ESP_LOGI(TAG, "Received API ACTION request.");

    char String[40];
    snprintf(String, sizeof(String), "SMS: Action by WEB API.\n");
    UART_send((const char *)String, sizeof(String));
    ESP_LOGI(TAG, "Sent: %s", String);

    return ESP_OK;
}