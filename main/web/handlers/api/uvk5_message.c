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

#include "helper/api.h"
#include "helper/http.h"
#include "app/uvk5.h"

static const char *TAG = "WEB/API/UVK5_MESSAGE";


/* Handles JSON requests and sends SMS with event ID, to test:

curl -d '{"content":"Hello!"}' http://192.168.4.1/api/uvk5_message */
esp_err_t API_UVK5_MESSAGE_Create(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((file_server_data *)(req->user_ctx))->scratch;
    int received = 0;

    // Read request content
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            httpd_json_resp_send(req, HTTPD_500, "Failed to process request");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    // Parse JSON
    cJSON *root = cJSON_ParseWithLength(buf, total_len);

    // Handle invalid JSON
    if (root == NULL)
    {
        ESP_LOGE(TAG, "Invalid JSON received");
        httpd_json_resp_send(req, HTTPD_500, "Invalid JSON received");
        // Free memory
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Handle invalid attr
    cJSON *attr = cJSON_GetObjectItem(root, "content");
    if (attr == NULL)
    {
        ESP_LOGE(TAG, "No \"content\" attr found in JSON");
        httpd_json_resp_send(req, HTTPD_500, "No \"content\" attr found in JSON");
        // Free memory, it handles both root and attr
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    char *message_content = attr->valuestring;
    uint8_t len = strlen(message_content);

    ESP_LOGI(TAG, "Received: %s", message_content);
    
    // Send message
    UVK5_SendMessage(message_content, len);

    // Free memory, it handles both root and attr
    cJSON_Delete(root);

    httpd_json_resp_send(req, HTTPD_200, "Message sent");

    return ESP_OK;
}