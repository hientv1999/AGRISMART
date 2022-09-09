#ifndef BATTERY_HPP_
#define BATTERY_HPP_
float ReadVoltageAnalogPin(int pin);
unsigned int getBatteryLevel();
unsigned int chargingCurrent();
float chargingPower();
bool batteryCharging();
#endif