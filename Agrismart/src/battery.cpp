#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include "OLED.hpp"
#include <Adafruit_SSD1306.h>
extern int CC;
extern int USB_PLUG;
Adafruit_ADS1115 ads;

bool turnOnADC(bool display_text){
    unsigned long start_time = millis();
    while (!ads.begin() && millis() - start_time <= 3000){
        if (display_text){
            printlnClearOLED(processText("Energy sensor failed").c_str(), WHITE, 1);
            delay(500);
        }
    }
    if (!ads.begin()){
        Serial.println("ADS1115 failed");
        return false;
    }
    // ads.setDataRate(0x00E0);
    ads.setGain(GAIN_EIGHT);    // use this gain for all power management
    Serial.println("ADS1115 OK");
    return true;
}


float ReadVoltageAnalogPin(int pin){
  return float(analogRead(pin))/4095*3.1+0.1;
}

float readExternalADC(uint8_t pin){
    float sum = 0;
    int zero_count = 0;
    float value;
    for (int i=0; i<10; i++){
        value = ads.computeVolts(ads.readADC_SingleEnded(pin));
        if (abs(value) < 0.00000000001){
            zero_count++;
        } else {
            sum += value;
        }
    }
    if (zero_count == 10){
        return 0;
    } else {
        return sum/(10-zero_count);
    }
}

float getBatteryVoltage(){
    float sum = 0;
    int zero_count = 0;
    float value;
    for (int i=0; i<10; i++){
        value = ads.computeVolts(ads.readADC_SingleEnded(1));
        if (abs(value) < 0.0020000001){
            zero_count++;
        } else {
            sum += value;
        }
    }
    if (zero_count == 10){
        return 0;
    } else {
        return 11*sum/(10-zero_count);
    }
}

unsigned int getBatteryLevel(){
    float batteryVolt = getBatteryVoltage();
    if (batteryVolt - 3.38 > 0){        // 100%
        return 100;
    } else if (batteryVolt - 3.35 > 0){ // 90% - 99%
        return 300 * batteryVolt - 915;
    } else if (batteryVolt - 3.33 > 0){ // 80% - 70%
        return 500 * batteryVolt - 1585;
    } else if (batteryVolt - 3.3 > 0){  // 70% - 80%
        return 333.33 * batteryVolt - 1030;
    } else if (batteryVolt - 3.26 > 0){ // 50% - 70%
        return 500 * batteryVolt - 1580;
    } else if (batteryVolt - 3.25 > 0){ // 40% - 50%
        return 1000 * batteryVolt - 3210;
    } else if (batteryVolt - 3.23 > 0){ // 30% - 40%
        return 500 * batteryVolt - 1585;
    } else if (batteryVolt - 3.2 > 0){  // 20% - 30%
        return 333.33 * batteryVolt - 1046.7;
    } else if (batteryVolt - 3.15 > 0){ // 14% - 20%
        return 120 * batteryVolt - 364;
    } else if (batteryVolt - 3.0 > 0){  // 9.5% - 14%
        return 30 * batteryVolt - 80.5;
    } else if (batteryVolt - 2.8 > 0){  // 5% - 9.5%
        return 22.5 * batteryVolt - 58;
    } else if (batteryVolt - 2.54 > 0){ // 0.5% - 5%
        return 17.308 * batteryVolt - 43.462;
    } else {                            // 0%
        return 0;
    }
}

float getsolarVoltage(){
    ads.setGain(GAIN_FOUR);
    float sum = 0;
    int zero_count = 0;
    float value;
    for (int i=0; i<10; i++){
        value = ads.computeVolts(ads.readADC_SingleEnded(3));
        if (abs(value) < 0.0020000001){
            zero_count++;
        } else {
            sum += value;
        }
    }
    ads.setGain(GAIN_EIGHT);
    if (zero_count == 10){
        return 0;
    } else {
        return 11*sum/(10-zero_count);
    }
}

float chargingCurrent(){      // in mA
    return readExternalADC(0)/0.015*1000; // positive mean charging
}

float chargingPower(){      // positive mean charging
    float batteryVolt = 11*readExternalADC(1);
    float batteryCurrent = readExternalADC(0)/0.015;
    return batteryVolt*batteryCurrent;
}

bool batteryCharging(){ // less than 10mA consider no charging
    // either solar or USB plug
    if (chargingCurrent() > 10){ 
        return true;
    } else {
        return false;
    }
}