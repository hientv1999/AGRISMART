#ifndef BLE_FUNCTIONS_HPP_
#define BLE_FUNCTIONS_HPP_
void turnOnBLE(std::string sensorName);
bool fastSetup(char (&wifiName)[32], char (&password)[63], char (&ip)[15], char (&name)[32], char (&location)[32], long &gmtOffset_sec);
std::string askForSSID();  // verify before proceed
std::string askForPass();
std::string askForIP();
std::string askForSensorName();
std::string askForSensorLocation();
long askForOffset();
void turnOffBLE();
#endif
