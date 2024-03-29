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

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <esp_err.h>
#include <esp_http_server.h>

#define WEBSOCKET_MESSAGE_MAX_LENGTH 200

esp_err_t WEBSOCKET_Handle(httpd_req_t *req);
void WEBSOCKET_Send(const char *tag, const char *format, ...);
void WEBSOCKET_Ping(void *pvParameters);

#endif
