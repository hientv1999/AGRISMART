#ifndef TEMPHUM_HPP_
#define TEMPHUM_HPP_
#include <AHT10.h>
#include <Wire.h>
bool turnOnTempHum();
String getTemperature();
String getHumidity();
#endif
