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
#include <Wire.h>
#include "error.hpp"
VL53L0X waterLevelSensor;
bool turnOnTOF(bool display_text){
    waterLevelSensor.setBus(&Wire1);
    waterLevelSensor.setTimeout(500);
    unsigned long start_time = millis();
    while (waterLevelSensor.init() != true && millis() - start_time <= 3000) {
        // TOF_error();
        if (display_text){
            printlnClearOLED(processText("Fail to initialize VL53L0X sensor").c_str(), WHITE, 1);
            delay(500);
        }
    }
    // ledcDetachPin(ERROR);
    if (waterLevelSensor.init() != true){
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
        waterLevelSensor.setMeasurementTimingBudget(200000);
        float waterPercentage = (waterLevelSensor.readRangeSingleMillimeters()-92)*100/WATER_LEVEL_MAX_MILIMETER;   // add the calibrated equation here
        return String(waterPercentage, 2);
    }
}

unsigned int soilMoisture(){
    return 0; // add the calibrated equation here
}

void pumpWater(unsigned int powerInPercentage){
    ledcSetup(PWMchannel, frequencyPWM, resolution);
    ledcAttachPin(WATER_PUMP, PWMchannel);
    ledcWrite(PWMchannel, 255*powerInPercentage/100);
}
