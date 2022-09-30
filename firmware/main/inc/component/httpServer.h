#pragma once
#include <esp_http_server.h>


/**
 * Doxygen Documentation example
 * 
 * @link https://www.cs.cmu.edu/~410/doc/doxygen.html
 * 
 * @author Bruna Medeiros
 * 
 * @brief Stop webserver
 * 
 * @param server Server handler
 * 
 * @return void
 * 
 * @note It is necessary to call that only if start_webserver was called before 
 */
void stopWebserver(httpd_handle_t server);


void startWebserver(char* toSendBuffer, size_t sizeBuffer);