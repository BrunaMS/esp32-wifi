/*
    This example shows how to use the All Channel Scan or Fast Scan to connect
    to a Wi-Fi network.

    In the Fast Scan mode, the scan will stop as soon as the first network matching
    the SSID is found. In this mode, an application can set threshold for the
    authentication mode and the Signal strength. Networks that do not meet the
    threshold requirements will be ignored.

    In the All Channel Scan mode, the scan will end only after all the channels
    are scanned, and connection will start with the best network. The networks
    can be sorted based on Authentication Mode or Signal Strength. The priority
    for the Authentication mode is:  WPA2 > WPA > WEP > Open
*/

#pragma once
#ifdef CONFIG_WIFI_DEVICE_MODE_RECEIVER
void wifiServiceReceiver(void* pvParameters);
#endif

#ifdef CONFIG_WIFI_DEVICE_MODE_TRANSMITTER
void wifiServiceTransmitter(void* pvParameters);
#endif