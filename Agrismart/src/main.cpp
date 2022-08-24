#include "TempHum.hpp"
#include "OLED.hpp"
#include <Adafruit_SSD1306.h>
#include "WiFi_functions.hpp"
#include "EEPROM_functions.hpp"
#include <EEPROM.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include "BLE_functions.hpp"
#include "battery.hpp"
#include "watering.hpp"
#define EEPROM_SIZE_TO_ACCESS 4096
unsigned int NUM_OF_DATATYPE = 6; //air temperature - air humidity - water level - battery level - charging current - watering
const uint64_t TIME_TO_UPDATE_IN_SEC = 300; //time between uploading data to LAMP server in second
#define I2C_SDA 25
#define I2C_SCL 26
/*This will be randomly created in mass production*/

/*-----------PIN DEFINITION---------------------*/
int HAPTIC = 19;
int BUTTON_OLED = 32;
int ERROR = 33;
int BATT_LEVEL = 34;
int CHAR_CUR = 35;
int  TOUCH = 13;
bool toggleDisplay;
uint16_t threshold_touch = 20;
RTC_DATA_ATTR unsigned int tap_num = 0;
RTC_DATA_ATTR uint64_t lastUpdate; //epoch time in second for last update
RTC_DATA_ATTR unsigned int watering = 0;


int getTouchValue(uint16_t pin){
  int value = 0;
  for (int i=0; i<10; i++){
    value+=touchRead(pin);
  }
  return value/=10;
}

void TSR(){}

void cycleOLED(){
  digitalWrite(HAPTIC, HIGH);
  digitalWrite(HAPTIC, LOW);
  toggleDisplay = true;
}

uint64_t Update(String serverName, char sensorName[], char sensorLocation[]){
  const char apiKeyValue[] = "TemperatureHumidity";
  uint64_t sinceLastUpdate = getTime() - lastUpdate;
  uint64_t time_sleep_left = TIME_TO_UPDATE_IN_SEC - sinceLastUpdate;
  if (sinceLastUpdate >= TIME_TO_UPDATE_IN_SEC) { // if it is time to update-
    String dataName[NUM_OF_DATATYPE] = {"Temperature", "Humidity", "WaterLevel", "BatteryLevel", "ChargingCurrent", "Watering"};
    String dataValue[NUM_OF_DATATYPE] = {getTemperature(), getHumidity(), String(waterLevelPercentage()), String(getBatteryLevel()), String(chargingCurrent()), "1"};
    if (sendDataLAMP(serverName.c_str(), sensorName, sensorLocation, apiKeyValue, dataName, dataValue, NUM_OF_DATATYPE)){
        lastUpdate = getTime();
        time_sleep_left = TIME_TO_UPDATE_IN_SEC;
    } else {
        time_sleep_left = 9;
    }
    
    watering = 0;
    //check soil
    if (soilMoisture() <= 40){
      pumpWater(30);
      watering = 1;
    }
    
  }
  return time_sleep_left;
}


float ReadVoltageAnalogPin(int pin){
  return float(analogRead(pin))/4095*3.1+0.1;
}

void setup()
{
  pinMode(HAPTIC, OUTPUT);
  pinMode(ERROR, OUTPUT);
  pinMode(HAPTIC, OUTPUT);
  Wire1.begin(I2C_SDA, I2C_SCL, 100000);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println("Wake up");
  EEPROM.begin(EEPROM_SIZE_TO_ACCESS);
  uint64_t time_sleep_left = TIME_TO_UPDATE_IN_SEC;
  char ssid[32] ={};
  char pass[63] = {};
  char IP[15] = {};
  char sensorName[32] = {};
  char sensorLocation[32] = {};
  long gmtOffset_sec = 0;
  String serverName = "";
  if (FirstSetup() == 0){                 // first time setup
    // turn on OLED, display welcome screen, turn on sensor, turn on BLE
    turnOnOLED();
    turnOnTempHum();
    turnOnTOF();
    turnOnBLE("Unnamed Agrismart");
    printWelcomeScreen();
    if (setupOption(true)){
      //fast setup
      while (!fastSetup(ssid, pass, IP, sensorName, sensorLocation)){
        delay(500);
        if (!setupOption(false)){//if switch to manual setup
          // show instructions for manual setup
          instructionToLightBlue();
          // ask for SensorName -> SSID -> Pass -> IP -> Sensor Location 
          strcpy(sensorName, askForSensorName().c_str());
          bool WiFi_connection = false;
          while (!WiFi_connection){
            strcpy(ssid, askForSSID().c_str());
            strcpy(pass, askForPass().c_str());
            WiFi_connection = turnOnWiFi(sensorName, true); 
          }
          strcpy(sensorLocation, askForSensorLocation().c_str());
          gmtOffset_sec = askForOffset();
          saveOffset(gmtOffset_sec);
        }
      }
    } else {
      // show instructions for manual setup
      instructionToLightBlue();
      // ask for SensorName -> SSID -> Pass -> IP -> Sensor Location 
      strcpy(sensorName, askForSensorName().c_str());
      bool WiFi_connection = false;
      while (!WiFi_connection){
        strcpy(ssid, askForSSID().c_str());
        strcpy(pass, askForPass().c_str());
        WiFi_connection = turnOnWiFi(sensorName, true); 
      }
      strcpy(sensorLocation, askForSensorLocation().c_str());
      gmtOffset_sec = askForOffset();
      saveOffset(gmtOffset_sec);
    }
    saveSSID(ssid, strlen(ssid));
    savePassword(pass, strlen(pass));
    saveIP(IP, strlen(IP));
    saveSensorName(sensorName, strlen(sensorName));
    saveSensorLocation(sensorLocation, strlen(sensorLocation));
    // force device to update to server to obtain local timezone
    lastUpdate = getTime() - TIME_TO_UPDATE_IN_SEC;
    serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
    turnOffBLE();
    turnOnWiFi(sensorName, true);
    printlnClearOLED("Setup done", WHITE, 1);
    unsigned int start_finish = millis();
    while (millis() - start_finish < 2500);
    turnOffOLED();
    FinishSetup();
  } else {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_TOUCHPAD : //touch pad
      {
        digitalWrite(HAPTIC, HIGH);
        digitalWrite(HAPTIC, LOW);
        turnOnOLED();
        if (turnOnTempHum() && turnOnTOF()){
          strcpy(sensorName, retrieveSensorName().c_str());
          strcpy(IP, retrieveIP(sensorName).c_str());
          turnOnWiFi(sensorName, true);
          strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
          gmtOffset_sec = retrieveOffset(sensorName);
          configTime(gmtOffset_sec, 0, "pool.ntp.org");
          serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
          Serial.println("Prepare to display");
          touchAttachInterrupt(TOUCH, cycleOLED , threshold_touch);
          toggleDisplay = false;
          // calculate battery level bitmap here
          unsigned int lastPressOLED = millis();
          while (millis() - lastPressOLED <= 15000){ //cycle through display mode
            switch (tap_num){
              case 0: //everything
                displayOverview();
              break;

              case 1: // big temp
                displayTemperature(); 
              break;

              case 2: // big hum
                displayHumidity();
              break;

              case 3: // water level
                displayWaterLevel();
              break;

              case 4: // batterry level
                displayBatteryLevel(); 
              break;

              case 5: // wifi signal
                displayWiFi(); 
              break;
              
              default:  // never get here, error occurs
                ESP.restart();  
            }
            delay(500);
            if (toggleDisplay){
              tap_num ++;
              if (tap_num >= 6){
                tap_num -= 6;
              }
              toggleDisplay = false;
              lastPressOLED = millis();
            }
          }
        } else {
          delay(5000);
        }       
        Serial.println("End of screen");
        turnOffOLED();
      }
    break;
    
    case ESP_SLEEP_WAKEUP_TIMER : //timer
      strcpy(sensorName, retrieveSensorName().c_str());
      turnOnWiFi(sensorName, false);
      //without below 3 lines, TempHum cannot be initialized
      pinMode(BUTTON_OLED, OUTPUT);
      digitalWrite(BUTTON_OLED, HIGH);
      delay(100);
      //turn sensors on
      turnOnTempHum();
      turnOnTOF();
      strcpy(IP, retrieveIP(sensorName).c_str());
      strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
      serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
      
    break;
    default:  // wakeup by reset
        turnOnOLED();
        printWelcomeScreen();
        strcpy(sensorName, retrieveSensorName().c_str());
        turnOnWiFi(sensorName, true);
        if (getTouchValue(TOUCH) < threshold_touch){
          // factory reset code
          unsigned int origin = millis();
          bool factory_reset = true;
          while (millis()- origin <= 1000){
            if (getTouchValue(TOUCH) > threshold_touch){
              factory_reset = false;
            }
          }
          if (factory_reset){
            strcpy(IP, retrieveIP(sensorName).c_str());
            strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
            serverName = "http://" + String(IP) + "/user/gardening/Agrismart/deleteTable.php";
            if (deleteTable(serverName, sensorName, sensorLocation)){
              factoryReset(false);
            } else {
              if (ManualFactoryReset()){
                factoryReset(true);
              }
            }
          }
        }
        
        time_sleep_left = TIME_TO_UPDATE_IN_SEC;
      break;
    }
  }
  
  if (String(IP) != "no server"){
    time_sleep_left = Update(serverName, sensorName, sensorLocation);
  }
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
  esp_sleep_enable_timer_wakeup(time_sleep_left*1000000-8000000); // delay average upon each sending 
  touchAttachInterrupt(TOUCH, TSR, threshold_touch);
  esp_sleep_enable_touchpad_wakeup();
  Serial.println("Sleep now");
  Serial.flush();
  esp_deep_sleep_start();
  //Wire.setClock(400000); //experimental I2C speed! 400KHz, default 100KHz
}
void loop(){}
