    #include <Arduino.h>
int BATT_LEVEL = 34;
int CHAR_CUR = 35;

float ReadVoltageAnalogPin(int pin){
  return float(analogRead(pin))/4095*3.1+0.1;
}

unsigned int getBatteryLevel(){
    return ReadVoltageAnalogPin(BATT_LEVEL)*32.3834;
}

unsigned int chargingCurrent(){    // in mA integer
    return ReadVoltageAnalogPin(CHAR_CUR) * 1000 / 3;
}

float chargingPower(){
    return ReadVoltageAnalogPin(BATT_LEVEL)*chargingCurrent()/1000;
}

bool batteryCharging(){ // less than 100mA consider no charging
    if (chargingCurrent() < 100){ 
        return false;
    } else {
        return true;
    }
}