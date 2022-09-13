#include <Arduino.h>
extern int CC;
extern int USB_PLUG;
float ReadVoltageAnalogPin(int pin){
  return float(analogRead(pin))/4095*3.1+0.1;
}

unsigned int getBatteryLevel(){
    return 0;
    // return ReadVoltageAnalogPin(BATT_LEVEL)*32.3834;
}

unsigned int chargingCurrent(){    // in mA integer
    return 0;
    // return ReadVoltageAnalogPin(CHAR_CUR) * 1000 / 3;
}

float chargingPower(){
    return 0;
    // return ReadVoltageAnalogPin(BATT_LEVEL)*chargingCurrent()/1000;
}

bool batteryCharging(){ // less than 100mA consider no charging
    // either solar or USB plug
    if (chargingCurrent() > 100 || (ReadVoltageAnalogPin(CC) -0.86 > 0 && ReadVoltageAnalogPin(USB_PLUG) > 2)){ 
        return true;
    } else {
        return false;
    }
}