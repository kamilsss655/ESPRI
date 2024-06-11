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

#include "router.h"
#include "helper/http.h"
#include "hardware/sd.h"
#include "web/handlers/root.h"
#include "web/handlers/websocket.h"
#include "web/handlers/static_files.h"
#include "web/handlers/api/audio.h"
#include "web/handlers/api/event.h"
#include "web/handlers/api/uvk5_message.h"
#include "web/handlers/api/settings.h"
#include "web/handlers/api/system.h"

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
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &root_uri);

    // Websocket
    httpd_uri_t websocket_uri = {
        .uri = "/websocket",
        .method = HTTP_GET,
        .handler = WEBSOCKET_Handle,
        .user_ctx = NULL,
        .is_websocket = true};
    httpd_register_uri_handler(server, &websocket_uri);

    // API System
    httpd_uri_t api_system_info_uri = {
        .uri = "/api/system/info",
        .method = HTTP_GET,
        .handler = API_SYSTEM_Info,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_system_info_uri);

    httpd_uri_t api_system_reboot_uri = {
        .uri = "/api/system/reboot",
        .method = HTTP_PUT,
        .handler = API_SYSTEM_Reboot,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_system_reboot_uri);

    httpd_uri_t api_system_deep_sleep_uri = {
        .uri = "/api/system/deep_sleep",
        .method = HTTP_PUT,
        .handler = API_SYSTEM_DeepSleep,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_system_deep_sleep_uri);

    httpd_uri_t api_system_factory_reset_uri = {
        .uri = "/api/system/factory_reset",
        .method = HTTP_PUT,
        .handler = API_SYSTEM_FactoryReset,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_system_factory_reset_uri);

    // API Settings
    httpd_uri_t api_settings_index_uri = {
        .uri = "/api/settings",
        .method = HTTP_GET,
        .handler = API_SETTINGS_Index,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_settings_index_uri);

    httpd_uri_t api_settings_create_uri = {
        .uri = "/api/settings",
        .method = HTTP_POST,
        .handler = API_SETTINGS_Create,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_settings_create_uri);

    // API Audio
    httpd_uri_t api_audio_record_uri = {
        .uri = "/api/audio/record",
        .method = HTTP_PUT,
        .handler = API_AUDIO_Record,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_audio_record_uri);

    // API Event
    httpd_uri_t api_event_create_uri = {
        .uri = "/api/event",
        .method = HTTP_POST,
        .handler = API_EVENT_Create,
        .user_ctx = server_data};
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
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &api_uvk5_message_create_uri);

    // File upload
    httpd_uri_t static_file_upload_uri = {
        .uri = UPLOAD_URI_PREFIX "/*",
        .method = HTTP_POST,
        .handler = STATIC_FILES_Upload,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &static_file_upload_uri);

    // Serve static files from the SD card
    httpd_uri_t static_file_download_from_sd_uri = {
        .uri = SD_BASE_PATH "/*", // Match all other URIs
        .method = HTTP_GET,
        .handler = STATIC_FILES_DownloadFromSD,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &static_file_download_from_sd_uri);

    // Match all and try to serve static files
    httpd_uri_t static_file_download_from_flash_uri = {
        .uri = "/*", // Match all other URIs
        .method = HTTP_GET,
        .handler = STATIC_FILES_DownloadFromFlash,
        .user_ctx = server_data};
    httpd_register_uri_handler(server, &static_file_download_from_flash_uri);
}