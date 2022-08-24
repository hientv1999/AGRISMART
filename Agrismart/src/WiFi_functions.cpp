#include "OLED.hpp"
#include "EEPROM_functions.hpp"
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <Arduino_JSON.h>

bool turnOnWiFi(char sensorName[], bool OLED){
    char ssid[EEPROM.read(1)] = {};
    strcpy(ssid, retrieveSSID(sensorName).c_str());
    char pass[EEPROM.read(2)] = {};
    strcpy(pass, retrievePassword(sensorName).c_str());
    char sensorLocation[EEPROM.read(5)] = {};
    strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
    WiFi.disconnect(true);
    WiFi.begin(ssid, pass);
    const char* processed_sensorName = processText(sensorName).c_str();
    if (OLED){
        printlnClearOLED(processed_sensorName, WHITE, 1);
        printlnOLED(sensorLocation, WHITE, 1);
        printlnOLED("Connecting", WHITE, 1);
    }
    int i = 0;
    unsigned int start = millis();
    while (WiFi.status() != WL_CONNECTED){
        if (OLED){
            printOLED(".", WHITE, 1);
        }
        delay(500);
        i++;
        if (i >= 3){
            if (OLED){
                printlnClearOLED(processed_sensorName, WHITE, 1);
                printlnOLED(sensorLocation, WHITE, 1);
                printlnOLED("Connecting", WHITE, 1);
            }
            i = 0;
        }
        if (millis() - start >= 10000){
            if (OLED){
                printClearOLED("Fail to connect to WiFi, try again the SSID and Password", WHITE, 1);
            }
            delay(5000);
            return false;
        }
    }
    return true;
}
bool sendDataLAMP(const char* serverName, const char* sensorName, const char* sensorLocation, const char* api_key, const String dataName[], const String dataValue[], const unsigned int sizeArray){
    if(WiFi.status()== WL_CONNECTED){
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "api_key=" + String(api_key) + "&sensor=" + String(sensorName) + "&location=" + String(sensorLocation);
        for (unsigned int i = 0; i < sizeArray; i++){
            httpRequestData += ("&" + dataName[i] + "=" + dataValue[i]) ;
        }
        int httpResponseCode = http.POST(httpRequestData);
        if (httpResponseCode != 200){
            Serial.println("Fail to send data to server");
            if (httpResponseCode>0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            return false;
        } else {
            // Get JSON data
            String json_data = http.getString();
            JSONVar myObject = JSON.parse(json_data);
            const char* offset = myObject["offset"];
            const char* resetDevice = myObject["reset"];
            if (offset == NULL || offset[0] == '\0'){
                Serial.println("Fail to send");
                return false;
            }
            Serial.println("Send successfully");
            long gmtOffset = 0;
            if (offset[0] == '-'){
                gmtOffset = atoi(&offset[1]) * -1;
            } else {
            gmtOffset = atoi(offset);
            }
            if (EEPROM.read(6) == 0 || retrieveOffset("Unnamed Agrismart") != gmtOffset){
                // Save to EEPROM
                saveOffset(gmtOffset);
            }
            // Free resources
            http.end();
            Serial.println("Check resetDevice");
            Serial.println(resetDevice);
            if (resetDevice != NULL){
                Serial.println("start reset");
                turnOnOLED();
                factoryReset(false);
            }
            return true;
        }
        
        
    } else if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected");
        return false;
    }
    return false;
}

bool deleteTable(String serverName, const char* sensorName, const char* sensorLocation){
    if(WiFi.status()== WL_CONNECTED){
        HTTPClient http;
        http.begin(serverName);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "api_key=TemperatureHumidity&sensorName=" + String(sensorName) + "&location=" + String(sensorLocation);
        int httpResponseCode = http.POST(httpRequestData);
        if (httpResponseCode != 200){
            printClearOLED(processText("Fail to send data to server").c_str(), WHITE, 1);
            if (httpResponseCode>0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            }
            else {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            return false;
        } else {
            Serial.println("Delete successfully");
            // Free resources
            http.end();
            return true;
        }
    } else if (WiFi.status() != WL_CONNECTED) {
        printClearOLED(processText("Factory reset fails due to no WiFi connection").c_str(), WHITE, 1);
        delay(5000);
        return false;
    }
    return false;
}

uint64_t getTime(){
   return uint64_t(time(NULL));
}

