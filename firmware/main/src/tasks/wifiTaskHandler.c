#include "cJSON.h"
#include "esp_log.h"
#include "esp_http_client.h"

#include "wifi.h"
#include "filesystem.h"
#include "httpClient.h"
#include "httpServer.h"
#include "wifiTaskHandler.h"

#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define TAG "WiFi Task"
/* FreeRTOS event group to signal when we are connected*/
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define SERVER_BUFFER_SIZE 10000

int s_retry_num = 0;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
EventGroupHandle_t wifiEventGroup = NULL;

static void wifiEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
#ifdef CONFIG_WIFI_DEVICE_MODE_RECEIVER
    ip_event_got_ip_t* ip_event = NULL;
#endif
    wifi_event_ap_staconnected_t* ap_staconnected_event = NULL;
    wifi_event_ap_stadisconnected_t* ap_stadisconnected_event = NULL;

    switch(event_id){
        case WIFI_EVENT_AP_STACONNECTED:
            ap_staconnected_event = (wifi_event_ap_staconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                    MAC2STR(ap_staconnected_event->mac), ap_staconnected_event->aid);
            break;
            
        case WIFI_EVENT_AP_STADISCONNECTED:
            ap_stadisconnected_event = (wifi_event_ap_stadisconnected_t*) event_data;
            ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                    MAC2STR(ap_stadisconnected_event->mac), ap_stadisconnected_event->aid);
            break;

#ifdef CONFIG_WIFI_DEVICE_MODE_RECEIVER
        case WIFI_EVENT_STA_START:
            if (event_base == WIFI_EVENT) {
                esp_wifi_connect();
            }
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            if(event_base == WIFI_EVENT) {
                if (s_retry_num < ESP_WIFI_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGI(TAG, "retry to connect to the AP");
                } else {
                    xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
                    ESP_LOGI(TAG,"connect to the AP fail");
                }
            }
            break;
        case IP_EVENT_STA_GOT_IP:
            if (event_base == IP_EVENT) {
                ip_event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&ip_event->ip_info.ip));
                s_retry_num = 0;
                xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
            }
            break;
#endif
    }
}

void wifiServiceReceiver(void* pvParameters){
    wifiEventGroup = xEventGroupCreate();
	wifiInitSta(wifiEventHandler);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(30000));

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s\n Requesting file to server",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
        httpRequestFile(FILE_NAME);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    
    for(;;){
        ESP_LOGD(TAG, "Inside task loop (receiver)...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void wifiServiceTransmitter(void* pvParameters){
    wifiEventGroup = xEventGroupCreate();
	wifiInitSoftap(wifiEventHandler);

    startWebserver();
    for(;;){
        ESP_LOGD(TAG, "Inside task loop (transmitter)...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}