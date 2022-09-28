#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "wifiTaskHandler.h"

void app_main(void)
{
    TaskHandle_t wifiHandle = NULL;
#ifdef CONFIG_WIFI_DEVICE_MODE_TRANSMITTER
    xTaskCreate(wifiServiceTransmitter, "wifiService", 5000, NULL, configMAX_PRIORITIES - 1, &wifiHandle);
#else
#ifdef CONFIG_WIFI_DEVICE_MODE_RECEIVER
    xTaskCreate(wifiServiceReceiver, "wifiService", 5000, NULL, configMAX_PRIORITIES - 1, &wifiHandle);
#endif
#endif
    vTaskDelay(pdMS_TO_TICKS(5));
    vTaskDelete(NULL);
}