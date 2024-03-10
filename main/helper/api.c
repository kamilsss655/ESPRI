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
#include "api.h"

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
