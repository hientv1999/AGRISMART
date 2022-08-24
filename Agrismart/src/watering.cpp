int WATER_LEVEL_MAX_MILIMETER = 200;
int SOIL_MOISTURE = 39;
int WATER_PUMP = 17;
const int frequencyPWM = 1000;
const int PWMchannel = 3;
const int resolution = 8;
extern int ERROR;
#include "OLED.hpp"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <VL53L0X.h>
#include <Wire.h>
#include "error.hpp"
VL53L0X waterLevelSensor;
bool turnOnTOF(){
    waterLevelSensor.setBus(&Wire1);
    waterLevelSensor.setTimeout(500);
    unsigned long start_time = millis();
    while (waterLevelSensor.init() != true && millis() - start_time <= 5000) {
        // TOF_error();
    }
    // ledcDetachPin(ERROR);
    if (waterLevelSensor.init() != true){
        printlnClearOLED(processText("Fail to initialize VL53L0X sensor").c_str(), WHITE, 1);
        Serial.println("VL53L0X failed");
        return false;
    }
    Serial.println("VL53L0X OK");
    return true;
}

unsigned int waterLevelPercentage(){
    waterLevelSensor.setMeasurementTimingBudget(200000);
    return (waterLevelSensor.readRangeSingleMillimeters()-92)*100/WATER_LEVEL_MAX_MILIMETER;   // add the calibrated equation here
}

unsigned int soilMoisture(){
    return analogRead(SOIL_MOISTURE); // add the calibrated equation here
}

void pumpWater(unsigned int powerInPercentage){
    ledcSetup(PWMchannel, frequencyPWM, resolution);
    ledcAttachPin(WATER_PUMP, PWMchannel);
    ledcWrite(PWMchannel, 255*powerInPercentage/100);
}
