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

#include <sys/param.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <cJSON.h>

#include "static_files.h"
#include "web/router.h"
#include "hardware/sd.h"
#include "helper/http.h"
#include "helper/api.h"
#include "helper/filesystem.h"
#include "board.h"

static const char *TAG = "WEB/STATIC_FILES";

// Send list of all files and directories
static esp_err_t list_directory_contents(httpd_req_t *req, const char *dirpath)
{
    char entrypath[FILE_PATH_MAX];
    char entrysize[16];

    struct dirent *entry;
    struct stat entry_stat;

    DIR *dir = opendir(dirpath);
    const size_t dirpath_len = strlen(dirpath);

    // Retrieve the base path of file storage to construct the full path
    strlcpy(entrypath, dirpath, sizeof(entrypath));

    cJSON *root = cJSON_CreateObject();
    cJSON *files = cJSON_CreateArray();
    cJSON *directories = cJSON_CreateArray();

    // Iterate over all files / folders
    while ((entry = readdir(dir)) != NULL)
    {

        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        if (stat(entrypath, &entry_stat) == -1)
        {
            ESP_LOGE(TAG, "Failed to stat: %s", entry->d_name);
            continue;
        }
        sprintf(entrysize, "%ld", entry_stat.st_size);

        cJSON *object = cJSON_CreateObject();

        if (entry->d_type == DT_DIR)
        {
            // it's a directory
            cJSON_AddStringToObject(object, "name", entry->d_name);
            cJSON_AddItemToArray(directories, object);
        }
        else
        {
            // it's a file
            cJSON_AddStringToObject(object, "name", entry->d_name);
            cJSON_AddStringToObject(object, "size", entrysize);
            cJSON_AddItemToArray(files, object);
        }
    }
    closedir(dir);

    cJSON_AddItemToObject(root, "files", files);
    cJSON_AddItemToObject(root, "directories", directories);

    // Generate JSON string
    const char *json_str = cJSON_Print(root);
    // Free memory, it handles all the objects belonging to root
    cJSON_Delete(root);

    httpd_resp_set_type(req, "application/json");
    // Send response
    httpd_resp_sendstr(req, json_str);

    return ESP_OK;
}

/// @brief Strips leading prexif from str text
/// @param str
/// @param prefix
static void strip_prefix(char *str, char *prefix)
{
    int uploadTextLength = strlen(prefix);

    if (strncmp(str, prefix, uploadTextLength) == 0)
    {
        memmove(str, str + uploadTextLength, strlen(str) - uploadTextLength + 1);
    }
}

static esp_err_t download_file(httpd_req_t *req, const char *base_path)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    const char *filename = get_path_from_uri(filepath, base_path,
                                             req->uri, sizeof(filepath));
    if (!filename)
    {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_json_resp_send(req, HTTPD_500, "Filename too long");
        return ESP_FAIL;
    }

    // If name has trailing '/', respond with directory contents
    if (filename[strlen(filename) - 1] == '/')
    {
        return list_directory_contents(req, filepath);
    }

    if (stat(filepath, &file_stat) == -1)
    {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_json_resp_send(req, HTTPD_404, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd)
    {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_json_resp_send(req, HTTPD_500, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
    set_content_type_from_file(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((file_server_data *)req->user_ctx)->scratch;
    size_t chunksize;
    do
    {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK)
            {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_json_resp_send(req, HTTPD_500, "Failed to send file");
                return ESP_FAIL;
            }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// Handler to download a file kept on flash
esp_err_t STATIC_FILES_DownloadFromFlash(httpd_req_t *req)
{
    char last_char = req->uri[strlen(req->uri) - 1];

    if (last_char == '/')
    {
        // if last char is a slash do not add base path
        // as it is likely needed for flash directory listing
        return download_file(req, "");
    }
    else
    {
        // otherwise serve files from FLASH_BASE_PATH as root URL
        return download_file(req, FLASH_BASE_PATH);
    }
}

// Handler to download a file kept on SD card
esp_err_t STATIC_FILES_DownloadFromSD(httpd_req_t *req)
{
    return download_file(req, "");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest)
    {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash)
    {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize)
    {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

// Set HTTP response content type according to file extension
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf"))
    {
        return httpd_resp_set_type(req, "application/pdf");
    }
    else if (IS_FILE_EXT(filename, ".html"))
    {
        return httpd_resp_set_type(req, "text/html");
    }
    else if (IS_FILE_EXT(filename, ".jpeg"))
    {
        return httpd_resp_set_type(req, "image/jpeg");
    }
    else if (IS_FILE_EXT(filename, ".ico"))
    {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    else if (IS_FILE_EXT(filename, ".css"))
    {
        return httpd_resp_set_type(req, "text/css");
    }
    else if (IS_FILE_EXT(filename, ".js"))
    {
        return httpd_resp_set_type(req, "text/javascript");
    }
    else if (IS_FILE_EXT(filename, ".wav"))
    {
        return httpd_resp_set_type(req, "audio/x-wav");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

esp_err_t STATIC_FILES_Upload(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;

    strcpy(filepath, req->uri);
    strip_prefix(filepath, UPLOAD_URI_PREFIX);

    /* Filename cannot have a trailing '/' */
    if (filepath[strlen(filepath) - 1] == '/')
    {
        ESP_LOGE(TAG, "Invalid filename : %s", filepath);
        httpd_json_resp_send(req, HTTPD_500, "Invalid filename");
        return ESP_OK;
    }

#ifdef UPLOAD_PREVENT_FILE_OVERWRITE
    if (stat(filepath, &file_stat) == 0)
    {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_json_resp_send(req, HTTPD_400, "File already exists");
        return ESP_OK;
    }
#endif

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE)
    {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_json_resp_send(req, HTTPD_400, "File size must be less than " MAX_FILE_SIZE_STR);
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_OK;
    }

#ifdef UPLOAD_PREVENT_FILE_OVERWRITE
    // Delete the file
    delete_file(filepath);
#endif

    fd = fopen(filepath, "w");
    if (!fd)
    {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_json_resp_send(req, HTTPD_500, "Failed to create file");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filepath);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf = ((file_server_data *)req->user_ctx)->scratch;
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0)
    {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            delete_file(filepath);

            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_json_resp_send(req, HTTPD_500, "Failed to receive file");
            return ESP_OK;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd)))
        {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            delete_file(filepath);

            ESP_LOGE(TAG, "File write failed!");
            /* Respond with 500 Internal Server Error */
            httpd_json_resp_send(req, HTTPD_500, "Failed to write file to storage");
            return ESP_OK;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");

    httpd_json_resp_send(req, HTTPD_200, "File upload complete");
    return ESP_OK;
}

// Delete file handler
esp_err_t STATIC_FILES_Delete(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    strcpy(filepath, req->uri);
    strip_prefix(filepath, DELETE_URI_PREFIX);

    esp_err_t ret = delete_file(filepath);

    if(ret == ESP_OK)
    {
        httpd_json_resp_send(req, HTTPD_200, "File deleted");
    }
    else if (ret == ESP_ERR_NOT_FOUND)
    {
        httpd_json_resp_send(req, HTTPD_404, "File not found");
    }
    else
    {
        httpd_json_resp_send(req, HTTPD_500, "Unknown error");
    }

    return ESP_OK;
}