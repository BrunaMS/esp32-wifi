#pragma once

#define MAX_FILE_NAME 64
#define SPIFFS_PATH "/storage"
#define FILE_NAME "data_config.txt"

void initFilesystem(void);

bool exists(char *filename);

bool deleteFile(char *filename);

int writeFile(char* filename, char* content, bool append);

int readFile(char* filename, char* buffer, int bufferSize);

bool getNextFilePart(char* filename, char* buffer, int bufferSize, int* len);
