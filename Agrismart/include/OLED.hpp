#ifndef OLED_HPP_
#define OLED_HPP_
#include <Arduino.h>
bool turnOnOLED();
void turnOffOLED();
std::string processText(const char* text);
void printArrayOLED(const String text[], const unsigned int sizeArray, const uint16_t color, const int16_t x_cursor, const int16_t y_cursor, const uint8_t size);
void printClearOLED(const char* text, const uint16_t color, const uint8_t size);
void printlnClearOLED(const char* text, const uint16_t color, const uint8_t size);
void printOLED(const char* text, const uint16_t color, const uint8_t size);
void printlnOLED(const char* text, const uint16_t color, const uint8_t size);
void printSingleCenterOLED(const char* text, int16_t y, const uint16_t color, const uint8_t size);
bool setupOption(bool firstTime);
void instructionToLightBlue();
void welcomeScreen();
void displayPlugInUSB_C(bool plug);
void displayWarningPlug();
void displayTurnOnAnimation();
void displayTurnOffAnimation();
void displayOverview(bool AHT10_alive, bool VL53L0X_alive, bool ADS1115_alive);
void displayTemperature(bool AHT10_alive);
void displayHumidity(bool AHT10_alive);
void displayWaterLevel(bool VL53L0X_alive);
void displayBatteryLevel(bool ADS1115_alive);
void displayWiFi();
bool ManualFactoryReset();
#endif
