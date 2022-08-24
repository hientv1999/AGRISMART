#ifndef EEPROM_FUNCTIONS_HPP_
#define EEPROM_FUNCTIONS_HPP_
/*
Location table:
- length of WiFi ssid (max is 32)
- length of WiFi password (max is 63)
- length of IP (max is 15)
- sensorName (max is 32)
- sensorLocation (max is 32)
-----------------------
0 : setup/working state
1 : ssid length
2 : password length
3 : IP length
4 : sensor name length
5 : sensor location length
6 : timezone offset set
10 -> 41   : WiFi ssid
42 -> 104  : WiFi password
105 -> 119 : Local IP of server
120 -> 151 : sensor name
152 -> 183 : sensor location
184 -> 187: timezone offset
*/
unsigned int FirstSetup();
void FinishSetup();
std::string retrieveSensorName();
std::string retrieveSSID(char sensorName[]);
std::string retrievePassword(char sensorName[]);
std::string retrieveIP(char sensorName[]);
std::string retrieveSensorLocation(char sensorName[]);
long retrieveOffset(char sensorName[]);
void saveSSID(const char text[], int length);
void savePassword(const char text[], int length);
void saveIP(const char text[], int length);
void saveSensorName(const char text[], int length);
void saveSensorLocation(const char text[], int length);
void saveOffset(const long offset);
void factoryReset(bool manual);
#endif
