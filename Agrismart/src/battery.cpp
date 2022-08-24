#include <Arduino.h>
extern int BATT_LEVEL;
extern int CHAR_CUR;
extern float ReadVoltageAnalogPin(int pin);

unsigned int getBatteryLevel(){
    return ReadVoltageAnalogPin(BATT_LEVEL)*32.3834;
}

unsigned int chargingCurrent(){    // in mA integer
    unsigned int result = ReadVoltageAnalogPin(CHAR_CUR) * 1000 / 3 ;
    return result;
}

bool batteryCharging(){ // less than 100mA consider no charging
    if (chargingCurrent() < 100){ 
        return false;
    } else {
        return true;
    }
}