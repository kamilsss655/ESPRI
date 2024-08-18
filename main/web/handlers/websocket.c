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
#include <cJSON.h>

#include "websocket.h"
#include "hardware/http_server.h"
#include "external/printf/printf.h"

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

void WEBSOCKET_Binary_Send(uint8_t *data, size_t len)
{
    if (len==0) return;
    httpd_ws_frame_t ws_pkt;

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = data;
    ws_pkt.len = len;
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
    ws_pkt.final = true;

    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(gHttpServerHandle, &fds, client_fds);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Got no clients");
        return;
    }

    for (int i = 0; i < fds; i++)
    {
        int client_info = httpd_ws_get_fd_info(gHttpServerHandle, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET)
        {
            httpd_ws_send_frame_async(gHttpServerHandle, client_fds[i], &ws_pkt);
            ESP_LOGI(TAG, "Sending binary to client %d", i);
        }
    }
}

void WEBSOCKET_Send(const char *tag, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    char *buffer = (char *)calloc(1, WEBSOCKET_MESSAGE_MAX_LENGTH);

    vsnprintf(buffer, WEBSOCKET_MESSAGE_MAX_LENGTH, format, va);

    va_end(va);

    httpd_ws_frame_t ws_pkt;

    // Create JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "tag", tag);
    cJSON_AddStringToObject(root, "message", buffer);

    // Generate JSON string
    const char *json_str = cJSON_Print(root);

    // Free memory, it handles all the objects belonging to root
    cJSON_Delete(root);
    free(buffer);

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)json_str;
    ws_pkt.len = strlen(json_str);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    static size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    size_t fds = max_clients;
    int client_fds[max_clients];

    esp_err_t ret = httpd_get_client_list(gHttpServerHandle, &fds, client_fds);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Got no clients");
        return;
    }

    for (int i = 0; i < fds; i++)
    {
        int client_info = httpd_ws_get_fd_info(gHttpServerHandle, client_fds[i]);
        if (client_info == HTTPD_WS_CLIENT_WEBSOCKET)
        {
            httpd_ws_send_frame_async(gHttpServerHandle, client_fds[i], &ws_pkt);
            ESP_LOGI(TAG, "Sending msg to client %d", i);
        }
    }
}

// Ping task
void WEBSOCKET_Ping(void *pvParameters)
{
    while (1)
    {
        WEBSOCKET_Send(TAG, "Ping");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}