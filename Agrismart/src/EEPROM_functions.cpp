#include <EEPROM.h>
#include "OLED.hpp"
#include "BLE_functions.hpp"
#include <Adafruit_SSD1306.h>
extern int TOUCH;
extern int getTouchValue(uint16_t pin);
extern int threshold_touch;
extern int ERROR;
unsigned int FirstSetup(){  // 0 means first setup, 1 means wake up
   return EEPROM.read(0);
}
void FinishSetup(){
   EEPROM.write(0, 1);
   EEPROM.commit();
}

void saveSSID(const char text[], int length){
    for (int i=0; i < length; i++){
        EEPROM.write(10+i, text[i]);
        EEPROM.commit();
    }
    EEPROM.write(1, length);
    EEPROM.commit();

}
void savePassword(const char text[], int length){
    for (int i=0; i < length; i++){
        EEPROM.write(42+i, text[i]);
        EEPROM.commit();
    }
    EEPROM.write(2, length);
    EEPROM.commit();
}
void saveIP(const char text[], int length){
    for (int i=0; i < length; i++){
        EEPROM.write(105+i, text[i]);
        EEPROM.commit();
    }
    EEPROM.write(3, length);
    EEPROM.commit();
}
void saveSensorName(const char text[], int length){
    for (int i=0; i < length; i++){
        EEPROM.write(120+i, text[i]);
        EEPROM.commit();
    }
    EEPROM.write(4, length);
    EEPROM.commit();
}
void saveSensorLocation(const char text[], int length){
    for (int i=0; i < length; i++){
        EEPROM.write(152+i, text[i]);
        EEPROM.commit();
    }
    EEPROM.write(5, length);
    EEPROM.commit();
}

void saveOffset(const long offset) {
    for (int i=0; i<4; i++){
        EEPROM.write(184+i, (offset >> i*8) & 0xFF);
        EEPROM.commit();
    }
    EEPROM.write(6, 1);
    EEPROM.commit();
}

std::string retrieveSensorName(){
    std::string result;
   int sensorName_length = EEPROM.read(4);
   if (sensorName_length != 0){
       for (int i = 0; i < sensorName_length; i++){
           result.push_back(EEPROM.read(120+i));
       }
   } else {
        printlnClearOLED(processText("Sensor name cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        char newName[] = "Unnamed Agrismart";
        turnOnBLE(newName);
        result = askForSensorName();
        saveSensorName(result.c_str(), strlen(result.c_str()));
   }
   return result;
}

std::string retrieveSSID(char sensorName[]){
    std::string result;
    int SSID_length = EEPROM.read(1);
    if (SSID_length != 0){
        for (int i = 0; i < SSID_length; i++){
            result.push_back(EEPROM.read(10+i));
        }
    } else {
        printlnClearOLED(processText("WiFi SSID cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        turnOnBLE(sensorName);
        result = askForSSID();
        saveSSID(result.c_str(), strlen(result.c_str()));
    }
    return result;
}
std::string retrievePassword(char sensorName[]){
    std::string result;
    int Password_length = EEPROM.read(2);
    if (Password_length != 0){
            for (int i = 0; i < Password_length; i++){
                result.push_back(EEPROM.read(42+i));
            }
    } else {
        printlnClearOLED(processText("WiFi Password cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        turnOnBLE(sensorName);
        result = askForPass();
        savePassword(result.c_str(), strlen(result.c_str()));
    }
    return result;
}
std::string retrieveIP(char sensorName[]){
    std::string result;
    int IP_length = EEPROM.read(3);
    if (IP_length != 0){
        for (int i = 0; i < IP_length; i++){
            result.push_back(EEPROM.read(105+i));
        }
    } else {
        printlnClearOLED(processText("Server IP cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        turnOnBLE(sensorName);
        result = askForIP();
        saveIP(result.c_str(), strlen(result.c_str()));
    }
    return result;
}

std::string retrieveSensorLocation(char sensorName[]){
    std::string result;
    int sensorLocation_length = EEPROM.read(5);
    if (sensorLocation_length != 0){
        for (int i = 0; i < sensorLocation_length; i++){
            result.push_back(EEPROM.read(152+i));
        }
    } else {
        printlnClearOLED(processText("Sensor location cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        turnOnBLE(sensorName);
        result = askForSensorLocation();
        saveSensorLocation(result.c_str(), strlen(result.c_str()));
    }
    return result;
}

long retrieveOffset(char sensorName[]){
    long result = 0;
    if (EEPROM.read(6) != 0){
        long four = EEPROM.read(184);
        long three = EEPROM.read(185);
        long two = EEPROM.read(186);
        long one = EEPROM.read(187);
        result = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
    } else {
        printlnClearOLED(processText("Local timezone cannot retrieve").c_str(), WHITE, 1);
        printlnOLED("Tap to start again", WHITE, 1);
        while (getTouchValue(TOUCH) > threshold_touch);
        turnOnBLE(sensorName);
        result = askForOffset();
        saveOffset(result);
    }
    return result;
}

void factoryReset(bool manual){
    printlnClearOLED("Factory Resetting", WHITE, 1);
    printlnOLED("", WHITE, 1);
    for (int i=0; i<= 187; i++){
        EEPROM.write(i, 0);
        EEPROM.commit();
        if (i % 9 == 0){
            printOLED("-", WHITE, 1);
        }
    }
    delay(100);
    printlnClearOLED("Factory Reset Done!", WHITE, 1);
    delay(3000);
    if (manual){
        printClearOLED(processText("You will have to manually delete the device on your Greenie Realm website to finish the factory reset").c_str(), WHITE, 1);
        delay(10000);
    }
    ESP.restart();
}
