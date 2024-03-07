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

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <esp_spiffs.h>
#include <esp_http_server.h>

#include "helper/http.h"
#include "web/handlers/root_handler.h"
#include "web/handlers/static_files_handler.h"
#include "web/handlers/api/event_handler.h"

void ROUTER_init(file_server_data *server_data, httpd_handle_t *server)
{
    /* URI handler for /  address */
    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_html_get_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &index_uri);

    /* URI handler for /api/action address */
    httpd_uri_t api_action_uri = {
        .uri = "/api/event",
        .method = HTTP_POST,
        .handler = api_action_post_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &api_action_uri);

    /* URI handler for getting uploaded files */
    httpd_uri_t file_download = {
        .uri = "/*", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = download_get_handler,
        .user_ctx = server_data // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);
}