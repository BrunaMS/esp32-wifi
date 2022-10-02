#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_http_server.h"
#include "filesystem.h"

#define TAG "Web Server"
#define PACKAGE_SIZE 2048

esp_err_t sendServerResponse(httpd_req_t *req, char *filename){
    esp_err_t err = ESP_OK;
    bool keepSending = true;
    char fileBuffer[PACKAGE_SIZE] = "";
    int attempts = 3, contentLength = 0;

    if(!exists(filename)){
        ESP_LOGE(TAG, "File doesn't exist: %s", filename);
        return ESP_FAIL;
    }

    // Read file in parts if its size is greater than FILE_PART_SIZE
    bool eof = !getNextFilePart(filename, fileBuffer, PACKAGE_SIZE, &contentLength);
    httpd_resp_set_type(req, "text/plain");

    do{
        ESP_LOGV(TAG, "Sending file part. Size: %d", contentLength);
        err = httpd_resp_send_chunk(req, fileBuffer, contentLength); 
        if(err == ESP_OK){
            keepSending = !eof;
            // If you try to read after find eof, it will be open again. Check to avoid it.
            if(keepSending){
                eof = !getNextFilePart(filename, fileBuffer, PACKAGE_SIZE, &contentLength);
            }
        }
        else{
            if(attempts-- <= 0){
                ESP_LOGE(TAG, "Error sending server response, trying to send this part again: %s", esp_err_to_name(err));
                // Wait to avoid many repeated attempts
                vTaskDelay(pdMS_TO_TICKS(1));
            }else if(!eof){
                eof = !getNextFilePart(filename, fileBuffer, PACKAGE_SIZE, &contentLength);
                ESP_LOGE(TAG, "Error sending server response, max attempts reached [%s]. Sending next...", esp_err_to_name(err));
            }else{
                keepSending = false;
                ESP_LOGE(TAG, "Error sending last part of server response [%s]. Finishing...", esp_err_to_name(err));
            }
        }
    }while(keepSending);
    err = httpd_resp_send_chunk(req, NULL, 0); 
    return err;
    return ESP_OK;
}

/* Our URI handler function to be called during GET /uri request */
esp_err_t fileHandler(httpd_req_t *req)
{
    char filename[MAX_FILE_NAME] = "";

    if (httpd_req_get_hdr_value_str(req, "filename", filename, MAX_FILE_NAME) == ESP_OK) {
        ESP_LOGI(TAG, "Found header => filename: %s", filename);
        return sendServerResponse(req, filename);
    }else{      
        ESP_LOGE(TAG, "Request failed. Missing header: filename");
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, NULL);
        return ESP_FAIL;
    }
}

/* URI handler structure for GET /uri */
httpd_uri_t uriGet = {
    .uri      = "/getFileContent",
    .method   = HTTP_GET,
    .handler  = fileHandler,
    .user_ctx = NULL,
};

/* Function for starting the webserver */
httpd_handle_t startWebserver()
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 10000;
    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uriGet);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stopWebserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}