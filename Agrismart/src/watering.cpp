int WATER_LEVEL_MAX_MILIMETER = 200;
extern int SOIL_MOISTURE;
extern int WATER_PUMP;
const int frequencyPWM = 1000;
const int PWMchannel = 3;
const int resolution = 8;
extern int ERROR;
#include "OLED.hpp"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <VL53L0X.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include "error.hpp"
#include "battery.hpp"
#include "EEPROM_functions.hpp"
extern Adafruit_ADS1115 ads;

extern VL53L0X waterLevelSensor;
bool turnOnTOF(bool display_text){
    waterLevelSensor.setBus(&Wire1);
    waterLevelSensor.setTimeout(500);
    unsigned long start_time = millis();
    while (!waterLevelSensor.init() && millis() - start_time <= 3000) {
        // TOF_error();
        if (display_text){
            printlnClearOLED(processText("Fail to initialize VL53L0X sensor").c_str(), WHITE, 1);
            delay(500);
        }
    }
    // ledcDetachPin(ERROR);
    if (!waterLevelSensor.init()){
        Serial.println("VL53L0X failed");
        return false;
    }
    Serial.println("VL53L0X OK");
    return true;
}

String waterLevelPercentage(bool VL53L0X_alive){
    if (!VL53L0X_alive){
        return "???";
    } else {
        uint16_t topLevel = retrieveTopWaterLevel();
        uint16_t bottomLevel = retrieveBottomWaterLevel();
        waterLevelSensor.setMeasurementTimingBudget(200000);
        float waterPercentage = (topLevel -waterLevelSensor.readRangeSingleMillimeters())*100/(topLevel - bottomLevel);   // add the calibrated equation here
        return String(waterPercentage, 2);
    }
}

float soilMoisture(){
    ads.setGain(GAIN_ONE);
    unsigned int moisture = ads.readADC_SingleEnded(2);
    ads.setGain(GAIN_EIGHT);  
    float moisture_level = moisture; // add the calibrated equation here
    return moisture_level; 
}

void pumpWater(unsigned int powerInPercentage){
    ledcSetup(PWMchannel, frequencyPWM, resolution);
    ledcAttachPin(WATER_PUMP, PWMchannel);
    ledcWrite(PWMchannel, 255*powerInPercentage/100);
}
