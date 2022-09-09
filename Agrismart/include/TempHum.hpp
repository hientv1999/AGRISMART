#ifndef TEMPHUM_HPP_
#define TEMPHUM_HPP_
#include <AHT10.h>
#include <Wire.h>
bool turnOnTempHum(bool display_text = true);
String getTemperature(bool AHT10_alive);
String getHumidity(bool AHT10_alive);
#endif
