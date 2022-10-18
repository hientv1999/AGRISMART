#ifndef BATTERY_HPP_
#define BATTERY_HPP_
bool turnOnADC(bool display_text = true);
float ReadVoltageAnalogPin(int pin);
float readExternalADC(uint8_t pin);
float getBatteryVoltage();
float solarVoltage();
float chargingCurrent();
float chargingPower();
bool batteryCharging();
unsigned int getBatteryLevel();
#endif