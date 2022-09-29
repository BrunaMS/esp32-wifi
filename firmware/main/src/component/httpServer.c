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

char resp[TEST_MAX_FILE_LENGTH] = "";

/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */

    int size = readFile(TEST_FILE_NAME, resp, TEST_MAX_FILE_LENGTH);
    
    httpd_resp_set_type(req, "text/plain");
    // char temp[1000] = "";
    char *pResp = resp;
    while(size > PACKAGE_SIZE){
        httpd_resp_send_chunk(req, pResp, PACKAGE_SIZE);
        pResp = pResp+PACKAGE_SIZE;
        size -=PACKAGE_SIZE;
    }
    httpd_resp_send_chunk(req, pResp, size);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/file_content",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

// /* URI handler structure for POST /uri */
// httpd_uri_t uri_post = {
//     .uri      = "/file_content",
//     .method   = HTTP_POST,
//     .handler  = post_handler,
//     .user_ctx = NULL
// };

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{

    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        // httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}