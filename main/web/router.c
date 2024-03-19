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
#include "web/handlers/root.h"
#include "web/handlers/static_files.h"
#include "web/handlers/api/event.h"
#include "web/handlers/api/uvk5_message.h"
#include "web/handlers/api/settings.h"

/* Initialize rounting for web requests

For REST resources use following name convention
GET    /      -> Index
POST   /      -> Create
GET    /:id   -> Show
PUT    /:id   -> Update
DELETE /:id   -> Destroy */
void ROUTER_Init(file_server_data *server_data, httpd_handle_t *server)
{
    // Root URL
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = ROOT_Handle,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &root_uri);

    // API Settings
    httpd_uri_t api_settings_index_uri = {
        .uri = "/api/settings",
        .method = HTTP_GET,
        .handler = API_SETTINGS_Index,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &api_settings_index_uri);

    httpd_uri_t api_settings_create_uri = {
        .uri = "/api/settings",
        .method = HTTP_POST,
        .handler = API_SETTINGS_Create,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &api_settings_create_uri);

    // API Event
    httpd_uri_t api_event_create_uri = {
        .uri = "/api/event",
        .method = HTTP_POST,
        .handler = API_EVENT_Create,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &api_event_create_uri);

    // API UVK5 Message
    // httpd_uri_t api_uvk5_message_index_uri = {
    //     .uri = "/api/uvk5_message",
    //     .method = HTTP_GET,
    //     .handler = API_UVK5_MESSAGE_Index,
    //     .user_ctx = server_data
    // };
    // httpd_register_uri_handler(server, &api_uvk5_message_index_uri);

    httpd_uri_t api_uvk5_message_create_uri = {
        .uri = "/api/uvk5_message",
        .method = HTTP_POST,
        .handler = API_UVK5_MESSAGE_Create,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &api_uvk5_message_create_uri);

    // Match all and try to serve static files
    httpd_uri_t static_file_uri = {
        .uri = "/*", // Match all other URIs
        .method = HTTP_GET,
        .handler = STATIC_FILES_Handle,
        .user_ctx = server_data
    };
    httpd_register_uri_handler(server, &static_file_uri);
}