/* ESP HTTP Client Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_http_client.h"

#include "freertos/event_groups.h"

#include "string.h"
#include "httpClient.h"
#include "filesystem.h"

#define TAG "Http Client"
#define CLIENT_RX_BUFFER_SIZE 2048
#define HTTPS_RECEIVED_DATA_BIT BIT0

char filename[MAX_FILE_NAME] = "";
EventGroupHandle_t clientEventGroup = NULL;
// uint32_t bufferSize = 0;
// uint32_t bufferAvailable = 0;

esp_err_t httpEventHandler(esp_http_client_event_t *evt)
{
    int messageSize = 0;
    static uint64_t totalSize = 0;
    char responseBuffer[CLIENT_RX_BUFFER_SIZE] = "";


    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA. event data len: %d", evt->data_len);
            messageSize = evt->data_len;
            strncpy(responseBuffer, (char *)evt->data, messageSize);
            if(messageSize > 0){
                totalSize += messageSize;
                ESP_LOGW(TAG, "Total of %llu: bytes read[%d].", totalSize, messageSize);
                writeFile(filename, responseBuffer, true);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            totalSize = 0;
            xEventGroupSetBits(clientEventGroup, HTTPS_RECEIVED_DATA_BIT);
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            totalSize = 0;
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}
 
void httpGetFile(char* serverUrl, char* path, char *filename){
    esp_err_t err = ESP_OK;
    esp_http_client_handle_t client = NULL;

    esp_http_client_config_t config = {
        .url = serverUrl,
        // .host = serverUrl,
        .path = path,
        // .user_data = responseBuffer,
        .event_handler = httpEventHandler,
        .buffer_size = CLIENT_RX_BUFFER_SIZE-5,
    };

    client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "filename", filename);
    err = esp_http_client_perform(client);

    esp_http_client_cleanup(client);
}

void httpRequestFile(char *file){   
    int responseReceived = 0;
    strncpy(filename, file, MAX_FILE_NAME);
    clientEventGroup = xEventGroupCreate();

    if(exists(filename)){
        ESP_LOGW(TAG, "File '%s' already exists. Deleting to create a new one", filename);
        deleteFile(filename);
    }

    do{
        ESP_LOGW(TAG, "Sending request to server... ");
        httpGetFile("http://192.168.15.6:8080/", "", filename);
        responseReceived = xEventGroupWaitBits(clientEventGroup, HTTPS_RECEIVED_DATA_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(120000)) & HTTPS_RECEIVED_DATA_BIT;
    }while(responseReceived == 0);

    ESP_LOGI(TAG, "EventGroup message received.");
    xEventGroupClearBits(clientEventGroup, HTTPS_RECEIVED_DATA_BIT);
    return;
}