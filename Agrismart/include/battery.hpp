#ifndef BATTERY_HPP_
#define BATTERY_HPP_
bool turnOnADC(bool display_text = true);
float ReadVoltageAnalogPin(int pin);
unsigned int getBatteryLevel();
float solarVoltage();
float chargingCurrent();
float chargingPower();
bool batteryCharging();
#endif