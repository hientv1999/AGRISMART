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
bool setupOption(bool firstTime);
void instructionToLightBlue();
void printWelcomeScreen();
void displayOverview(bool AHT10_alive, bool VL53L0X_alive);
void displayTemperature(bool AHT10_alive);
void displayHumidity(bool AHT10_alive);
void displayWaterLevel(bool VL53L0X_alive);
void displayBatteryLevel();
void displayWiFi();
bool ManualFactoryReset();
#endif
