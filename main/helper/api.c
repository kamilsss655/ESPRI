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

#include <cJSON.h>
#include <esp_log.h>

#include "api.h"
#include "http.h"

// Responds with JSON
// for example: {"response": "Invalid JSON received"}
esp_err_t httpd_json_resp_send(httpd_req_t *req, const char *status, const char *content)
{
    // Set response params
    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");

    // Create response JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "response", content);
    const char *json_response = cJSON_Print(root);

    // Respond
    httpd_resp_sendstr(req, json_response);

    // Free memory
    free((void *)json_response);
    cJSON_Delete(root);

    return ESP_OK;
}

/// @brief Sets attributes based on JSON request and defined ApiAttr_t
/// @param req HTTPD request data structure
/// @param TAG TAG for logging
/// @param attr ApiAttr_t that matches JSON attributes to application specific memory locations
/// @return
esp_err_t process_api_attributes(httpd_req_t *req, const char *TAG, ApiAttr_t *api_attribute, size_t num_attributes)
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

    // Get all settings from the JSON object
    for (u_int8_t i = 0; i < num_attributes; i++)
    {
        cJSON *attr = cJSON_GetObjectItem(root, api_attribute[i].attr);
        // if attr is null we go to next item
        if (attr == NULL)
            continue;
        // set the setting based on the data type
        if (api_attribute[i].isInteger)
        {
            *(API_INTEGER_TYPE *)api_attribute[i].val = cJSON_GetNumberValue(attr);
        }
        else
        {
            strcpy((char *)api_attribute[i].val, cJSON_GetStringValue(attr));
        }
    }
    // Free memory, it handles both root and attr
    cJSON_Delete(root);

    return ESP_OK;
}