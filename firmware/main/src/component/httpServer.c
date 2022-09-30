#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_http_server.h"
#include "filesystem.h"

#define TAG "Web Server"
#define PACKAGE_SIZE 512

size_t size = 0;
char *pContentBuffer = NULL;

/* Our URI handler function to be called during GET /uri request */
esp_err_t getHandler(httpd_req_t *req)
{
    /* Send a simple response */
    if(pContentBuffer){
        char *pTmpBuffer = pContentBuffer;
        httpd_resp_set_type(req, "text/plain");
        while(size > PACKAGE_SIZE){
            httpd_resp_send_chunk(req, pTmpBuffer, PACKAGE_SIZE);
            pTmpBuffer = pTmpBuffer+PACKAGE_SIZE;
            size -=PACKAGE_SIZE;
        }
        httpd_resp_send_chunk(req, pTmpBuffer, size);
        return ESP_OK;
    }
    ESP_LOGE(TAG, "Error getting content pointer");
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* URI handler structure for GET /uri */
httpd_uri_t uriGet = {
    .uri      = "/getFileContent",
    .method   = HTTP_GET,
    .handler  = getHandler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t startWebserver(char* toSendBuffer, size_t sizeBuffer)
{
    pContentBuffer = toSendBuffer;
    size = sizeBuffer;
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

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