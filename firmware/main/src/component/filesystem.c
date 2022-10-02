/* SPIFFS filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "filesystem.h"

#define TAG "filesystem"
#define SPIFFS_PATH "/storage"
#define MAX_FILE_NAME 64

bool deleteFile(char *filename){
    char path[MAX_FILE_NAME] = "";
    snprintf(path,  MAX_FILE_NAME, "%s/%s", SPIFFS_PATH, filename);
    return (remove(path) == 0);
}

bool exists(char *filename){
    struct stat buffer;   
    char path[MAX_FILE_NAME] = "";
    
    snprintf(path,  MAX_FILE_NAME, "%s/%s", SPIFFS_PATH, filename);
    return (stat(path, &buffer) == 0);
}

int writeFile(char* filename, char* content, bool append){
    char path[MAX_FILE_NAME] = "";
    FILE* f = NULL;
    
    snprintf(path, MAX_FILE_NAME, "%s/%s", SPIFFS_PATH, filename);
    f = fopen(path, append?"a":"w");
    
    ESP_LOGI(TAG, "Writing on file");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return -1;
    }
    fprintf(f, content);
    fclose(f);
    return 0; 
}


bool getNextFilePart(char* filename, char* buffer, int bufferSize, int* len){
    bool eof = false;
    static FILE *f = NULL;
    char path[MAX_FILE_NAME] = "";
    
    if(f == NULL){
        ESP_LOGW(TAG, "File '%s' is not open yet. Doing that now.", filename);
        snprintf(path, MAX_FILE_NAME, "%s/%s", SPIFFS_PATH, filename);
        f = fopen(path, "r");
    }
    char c = '\0';
    int size = 0;
    while(!eof && (size < bufferSize)){
        c = fgetc(f);
        eof = feof(f);
        if(!eof){
            buffer[size] = c;
            size++;
        }
    }
    *len = size;
    if(eof){
        fclose(f);
        f = NULL;
        return false;
    }
    buffer[size] = '\0';
    return true;
}

int readFile(char* filename, char* buffer, int bufferSize){
    char path[MAX_FILE_NAME] = "";
    FILE* f = NULL;
    
    snprintf(path, MAX_FILE_NAME, "%s/%s", SPIFFS_PATH, filename);
    f = fopen(path, "r");
    
    ESP_LOGI(TAG, "Reading file");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return -1;
    }

    char c = 'a';
    int size = 0;
    while(!feof(f) && (size < bufferSize)){
        c = fgetc(f);
        if(!feof(f)){
            buffer[size] = c;
            size++;
        }
    }
    if(!feof(f)){
        ESP_LOGW(TAG, "Reached buffer size before eof");
    }

    fclose(f);
    ESP_LOGI(TAG, "Read from file: '%s'", buffer);
    return size;
}

void initFilesystem(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = SPIFFS_PATH,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

#ifdef CONFIG_SPIFFS_CHECK_ON_START
    ESP_LOGI(TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    } else {
        ESP_LOGI(TAG, "SPIFFS_check() successful");
    }
#endif

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }


    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return;
        } else {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }
    }
}