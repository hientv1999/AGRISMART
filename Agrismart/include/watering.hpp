#ifndef WATERING_HPP_
#define WATERING_HPP_
bool turnOnTOF(bool display_text = true);
String waterLevelPercentage(bool VL53L0X_alive);
unsigned int soilMoisture();
void pumpWater(unsigned int powerInPercentage);
#endif