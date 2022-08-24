#ifndef WATERING_HPP_
#define WATERING_HPP_
bool turnOnTOF();
unsigned int waterLevelPercentage();
unsigned int soilMoisture();
void pumpWater(unsigned int powerInPercentage);
#endif