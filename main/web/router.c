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

#include "helper/http.h"
#include "web/handlers/root_handler.h"
#include "web/handlers/static_files_handler.h"
#include "web/handlers/api/event_handler.h"

// Initialize rounting for web requests
void ROUTER_Init(file_server_data *server_data, httpd_handle_t *server)
{
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &root_uri);

    httpd_uri_t api_event_uri = {
        .uri = "/api/event",
        .method = HTTP_POST,
        .handler = api_event_handler,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &api_event_uri);

    httpd_uri_t static_file_uri = {
        .uri = "/*", // Match all other URIs
        .method = HTTP_GET,
        .handler = static_file_handler,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &static_file_uri);
}