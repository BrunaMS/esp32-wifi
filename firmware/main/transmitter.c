#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "wifiDriver.h"

void app_main(void)
{
    TaskHandle_t wifiHandle = NULL;

    xTaskCreate(wifiServiceTransmitter, "wifiService", 5000, NULL, configMAX_PRIORITIES - 1, &wifiHandle);
    vTaskDelay(pdMS_TO_TICKS(5));
    vTaskDelete(NULL);
}