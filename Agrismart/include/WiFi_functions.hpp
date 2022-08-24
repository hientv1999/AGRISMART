#ifndef WIFI_FUNCTIONS_HPP_
#define WIFI_FUNCTIONS_HPP_
#include <Arduino.h>
bool turnOnWiFi(char sensorName[], bool OLED);
// return WiFi Connection: True/False = Connected/Disconnected
bool sendDataLAMP(const char* serverName, const char* sensorName, const char* sensorLocation, const char* api_key, const String dataType[], const String dataValue[], const unsigned int sizeArray);
bool deleteTable(String serverName, const char* sensorName, const char* sensorLocation);
uint64_t getTime();

#endif
