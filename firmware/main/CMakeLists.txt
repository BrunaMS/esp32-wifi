idf_component_register(SRCS "main.c" "src/tasks/wifiTaskHandler.c" "inc/tasks/wifiTaskHandler.h" "src/component/wifi.c" "inc/component/wifi.h" "src/component/httpServer.c" "inc/component/httpServer.h" "src/component/filesystem.c" "inc/component/filesystem.h"  "src/component/httpClient.c" "inc/component/httpClient.h" 
                       INCLUDE_DIRS "." "inc/tasks" "src/tasks" "inc/component" "src/component"
                       REQUIRES esp_http_server esp_http_client nvs_flash spiffs esp-tls json)
