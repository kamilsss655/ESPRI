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
#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>

#include "helper/api.h"
#include "helper/http.h"
#include "app/uvk5.h"

static const char *TAG = "WEB/API/EVENT";

/*
Handles JSON requests and sends SMS with event ID, to test:

    curl --header "Content-Type: application/json" \
                   --request POST \
                   --data '{"id":231}' \
                   http://192.168.4.1/api/event
*/
esp_err_t API_EVENT_Create(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((file_server_data *)(req->user_ctx))->scratch;
    int received = 0;

    // TODO: below should probably be set at some higher level protection
    //  if (total_len >= SCRATCH_BUFSIZE) {
    //      /* Respond with 500 Internal Server Error */
    //      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
    //      return ESP_FAIL;
    //  }

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
    cJSON *attr = cJSON_GetObjectItem(root, "id");
    if (attr == NULL)
    {
        ESP_LOGE(TAG, "No \"id\" attr found in JSON");
        httpd_json_resp_send(req, HTTPD_500, "No \"id\" attr found in JSON");
        // Free memory, it handles both root and attr
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    int event_id = attr->valueint;
    // Free memory, it handles both root and attr
    cJSON_Delete(root);

    ESP_LOGI(TAG, "Event id:%d received", event_id);

    // Send SMS
    char String[30];
    snprintf(String, sizeof(String), "API event: %d received.", event_id);
    UVK5_SendMessage(String, sizeof(String));
    
    httpd_json_resp_send(req, HTTPD_200, "Event received");

    return ESP_OK;
}