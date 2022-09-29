#pragma once

#define TEST_MAX_FILE_LENGTH 10000
#define TEST_FILE_NAME "test_file.txt"

void initFilesystem(void);

int writeFile(char* filename, char* content);

int readFile(char* filename, char* buffer, int bufferSize);
