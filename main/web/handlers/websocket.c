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
#include <esp_log.h>
#include <esp_http_server.h>

#include "hardware/http_server.h"

static const char *TAG = "WEB/WEBSOCKET";

esp_err_t WEBSOCKET_Handle(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    return ESP_OK;
}

void WEBSOCKET_Send()
{
    httpd_ws_frame_t ws_pkt;

    ESP_LOGI(TAG, "WEBSOCKET_Send init");

    char buff[30];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "hello from websocket");
    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)buff;
    ws_pkt.len = strlen(buff);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(gHttpServerHandle, &fds, client_fds);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Got no clients");
        return;
    }

    for (int i = 0; i < fds; i++) {
        int client_info = httpd_ws_get_fd_info(gHttpServerHandle, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(gHttpServerHandle, client_fds[i], &ws_pkt);
            ESP_LOGI(TAG, "Sending msg to client %d", i);
        }
    }
}