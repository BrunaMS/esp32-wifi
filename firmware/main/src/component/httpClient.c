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

#define TAG "Http Client"
#define MAX_HTTP_RECV_BUFFER 2048
#define MAX_HTTP_OUTPUT_BUFFER 2048

#define HTTPS_RECEIVED_DATA_BIT BIT0

EventGroupHandle_t httpEventGroup = NULL;
uint32_t bufferSize = 0;
uint32_t bufferAvailable = 0;

esp_err_t httpEventHandler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGW(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            printf("\n\nevt->user_data\n%s\n\n",(char *)evt->data);
            if (evt->user_data) {
                strncpy(evt->user_data + bufferSize, evt->data, bufferAvailable);
            }
            bufferSize += evt->data_len;
            bufferAvailable -= evt->data_len;

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            bufferSize = 0;
            xEventGroupSetBits(httpEventGroup, HTTPS_RECEIVED_DATA_BIT);
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            bufferSize = 0;
            break;
    }
    return ESP_OK;
}
 
void httpGet(char* url, char *responseBuffer, size_t bufferSize)
{
    esp_http_client_config_t config = {
        .url = url,
        // .host = serverUrl,
        // .path = path,
        // .user_data = responseBuffer,
        .event_handler = httpEventHandler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP chunk encoding Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
                int sizeRead = esp_http_client_read_response(client, responseBuffer, bufferSize);
                printf("##### RESPONSE[%d]: \n%s\n\n", sizeRead, responseBuffer);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

void httpRequestFile(char* fileBuffer, size_t bufferSize)
{   
    int responseReceived = 0;
    httpEventGroup = xEventGroupCreate();
    
    do{
        bufferAvailable = bufferSize;
        ESP_LOGW(TAG, "Sending request to server... ");
        // http_get("http://192.168.4.1/getFileContent");
        httpGet("http://httpbin.org/robots.txt", "", fileBuffer, bufferSize);
        responseReceived = xEventGroupWaitBits(httpEventGroup, HTTPS_RECEIVED_DATA_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000)) & HTTPS_RECEIVED_DATA_BIT;
    }while(responseReceived == 0);

    xEventGroupClearBits(httpEventGroup, HTTPS_RECEIVED_DATA_BIT);
    return;
}