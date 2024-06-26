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

#include <esp_log.h>
#include <esp_http_server.h>

#include "http_server.h"
#include "helper/http.h"
#include "web/router.h"

static const char *TAG = "HW/HTTP_SERVER";

httpd_handle_t gHttpServerHandle = NULL;

/* Function to start the file server */
esp_err_t HTTP_SERVER_Init(const char *base_path)
{
    static file_server_data *server_data = NULL;

    if (server_data)
    {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(file_server_data));
    if (!server_data)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Increase max amount of URI handlers defined in router.c
    config.max_uri_handlers = HTTP_SERVER_MAX_URI_HANDLERS;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&gHttpServerHandle, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    // Initalize router
    ROUTER_Init(server_data, gHttpServerHandle);

    return ESP_OK;
}
