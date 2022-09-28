#include "wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "WiFi Task"

void wifiServiceReceiver(void* pvParameters){
	wifiInitSta();
    for(;;){
        ESP_LOGD(TAG, "Inside task loop (receiver)...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void wifiServiceTransmitter(void* pvParameters){
	wifiInitSoftap();
    for(;;){
        ESP_LOGD(TAG, "Inside task loop (transmitter)...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}