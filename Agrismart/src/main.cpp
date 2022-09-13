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
#include "error.hpp"
#define EEPROM_SIZE_TO_ACCESS 4096
unsigned int NUM_OF_DATATYPE = 6; //air temperature - air humidity - water level - battery level - charging current - watering
const uint64_t TIME_TO_UPDATE_IN_SEC = 300; //time between uploading data to LAMP server in second
#define I2C_SDA 25 
#define I2C_SCL 26 
#define BUTTON_PIN_BITMASK 0x8000000000 // 2^39 in hex
/*This will be randomly created in mass production*/

/*-----------PIN DEFINITION---------------------*/
int TOUCH = 13;
int NIGHT_LIGHT = 16;
int WATER_PUMP = 17;
int ENABLE_BOOST = 18;
int HAPTIC = 19;
int DISABLE_CHARGE = 23;
int BUTTON_OLED = 32;
int ERROR = 33;
int CC = 34;
int USB_PLUG = 39;
/*-----------SESSION SETTING---------------------*/
bool toggleDisplay;
bool togglePlug;
uint16_t threshold_touch = 20;
RTC_DATA_ATTR unsigned int tap_num = 0;
RTC_DATA_ATTR uint64_t lastUpdate; //epoch time in second for last update
RTC_DATA_ATTR unsigned int watering = 0;
RTC_DATA_ATTR bool charging_plug = false;

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
    String dataValue[NUM_OF_DATATYPE] = {String(getTemperature(true)), String(getHumidity(true)), String(waterLevelPercentage(true)), String(getBatteryLevel()), String(chargingCurrent()), "1"};
    if (sendDataLAMP(serverName.c_str(), sensorName, sensorLocation, apiKeyValue, dataName, dataValue, NUM_OF_DATATYPE)){
      lastUpdate = getTime();
    } else {
      Serial.println("server error LED blinking");
      server_error();
    }
    time_sleep_left = TIME_TO_UPDATE_IN_SEC;
    watering = 0;
    //check soil
    if (soilMoisture() <= 40){
      pumpWater(30);
      watering = 1;
    }
    
  }
  return time_sleep_left;
}


void setup()
{
  pinMode(HAPTIC, OUTPUT);
  pinMode(ERROR, OUTPUT);
  pinMode(DISABLE_CHARGE, OUTPUT);
  pinMode(BUTTON_OLED, OUTPUT);
  pinMode(NIGHT_LIGHT, OUTPUT);
  Wire1.begin(I2C_SDA, I2C_SCL, 100000); 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println("Wake up");
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0){  // USB is plugged/unplugged
    Serial.println("USB Type-C state changes");
    turnOnOLED();
    float CC_volt = ReadVoltageAnalogPin(CC);
    if (CC_volt - 0.33 > 0 && CC_volt - 0.86 < 0){  // USB cable not supply enough current
      charging_plug = true;
      digitalWrite(DISABLE_CHARGE, HIGH);
      displayWarningPlug();
      Serial.println("Not enough current");
    } else if (CC_volt - 0.86 > 0){                 // enough current
      charging_plug = true;
      digitalWrite(DISABLE_CHARGE, LOW);
      Serial.println("Start charging");
      displayPlugInUSB_C(true);
    } else {                                        // unplug cable
      charging_plug = false;
      digitalWrite(DISABLE_CHARGE, LOW);
      Serial.println("Stop charging");
      displayPlugInUSB_C(false);
    }
    turnOffOLED();
    if (charging_plug){
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);
      esp_sleep_enable_timer_wakeup(0xFFFFFFFFFFFFFFFF);    // sleep for infinite
      touchAttachInterrupt(TOUCH, TSR, threshold_touch);
      esp_sleep_enable_touchpad_wakeup();
      
      Serial.println("Charge sleep now");
      Serial.flush();
      esp_deep_sleep_start(); 
    }
  } 
  EEPROM.begin(EEPROM_SIZE_TO_ACCESS);
  uint64_t time_sleep_left = TIME_TO_UPDATE_IN_SEC;
  char ssid[32] ={};
  char pass[63] = {};
  char IP[15] = {};
  char sensorName[32] = {};
  char sensorLocation[32] = {};
  long gmtOffset_sec = 0;
  String serverName = "";
  bool AHT10_alive = false;
  bool VL53L0X_alive = false;
  if (FirstSetup() == 0){                 // first time setup
    // turn on OLED, display welcome screen, turn on sensor, turn on BLE
    turnOnOLED();
    turnOnBLE("Unnamed Agrismart");
    welcomeScreen();
    AHT10_alive = turnOnTempHum();
    VL53L0X_alive = turnOnTOF();
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
    printlnClearOLED("Finish Fast Setup", WHITE, 1);
    delay(1000);
    saveSSID(ssid, strlen(ssid));
    savePassword(pass, strlen(pass));
    saveIP(IP, strlen(IP));
    saveSensorName(sensorName, strlen(sensorName));
    saveSensorLocation(sensorLocation, strlen(sensorLocation));
    printlnClearOLED("Finish Saving", WHITE, 1);
    delay(1000);
    // force device to update to server to obtain local timezone
    lastUpdate = getTime() - TIME_TO_UPDATE_IN_SEC;
    serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
    turnOffBLE();
    printlnClearOLED("BLE off", WHITE, 1);
    delay(1000);
    turnOnWiFi(sensorName, true);
    printlnClearOLED("Setup done", WHITE, 1);
    unsigned int start_finish = millis();
    while (millis() - start_finish < 2500);
    turnOffOLED();
    FinishSetup();
  } else {
    switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_TOUCHPAD : //touch pad
      {
        digitalWrite(HAPTIC, HIGH);
        delay(10);
        digitalWrite(HAPTIC, LOW);
        // read SOLAR_VOLT < 1V then pull NIGHT_LIGHT high, at the end pull NIGHT_LIGHT low
        if (turnOnOLED()){
          displayTurnOnAnimation();
          AHT10_alive = turnOnTempHum();
          VL53L0X_alive = turnOnTOF();
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
          togglePlug = false;
          float CC_volt;
          float USB_PLUG_volt;
          unsigned int lastPressOLED = millis();
          while (millis() - lastPressOLED <= 15000){ //cycle through display mode
            // USB screen if plug/unplug
            USB_PLUG_volt = ReadVoltageAnalogPin(USB_PLUG);
            if ((USB_PLUG_volt > 2 && charging_plug == false) || (USB_PLUG_volt < 2 && charging_plug == true)){
              if (USB_PLUG_volt > 2){
                charging_plug = true;
              } else {
                charging_plug= false;
              }
              CC_volt = ReadVoltageAnalogPin(CC);
              if (CC_volt - 0.33 > 0 && CC_volt - 0.86 < 0){  // USB cable not supply enough current
                digitalWrite(DISABLE_CHARGE, HIGH);
                displayWarningPlug();
                Serial.println("Not enough current");
              } else {                                        // enough current or no connection
                digitalWrite(DISABLE_CHARGE, LOW);
                displayPlugInUSB_C(charging_plug);
              }
            }
            // normal screen
            switch (tap_num){
              case 0: //everything
                displayOverview(AHT10_alive, VL53L0X_alive);
              break;

              case 1: // big temp
                displayTemperature(AHT10_alive); 
              break;

              case 2: // big hum
                displayHumidity(AHT10_alive);
              break;

              case 3: // water level
                displayWaterLevel(VL53L0X_alive);
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
            if (toggleDisplay){
              tap_num ++;
              if (tap_num >= 6){
                tap_num -= 6;
              }
              toggleDisplay = false;
              lastPressOLED = millis();
            }
            delay(500);
          }    
          Serial.println("End of screen");
          displayTurnOffAnimation();
          turnOffOLED();
        }
        detachInterrupt(USB_PLUG);
      }
    break;
    
    case ESP_SLEEP_WAKEUP_TIMER : //timer
      strcpy(sensorName, retrieveSensorName().c_str());
      turnOnWiFi(sensorName, false);
      //turn sensors on
      AHT10_alive = turnOnTempHum(false);
      VL53L0X_alive = turnOnTOF(false);
      strcpy(IP, retrieveIP(sensorName).c_str());
      strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
      serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
    break;
    
    case ESP_SLEEP_WAKEUP_EXT0:
    break;

    default:  // wakeup by reset
        if (turnOnOLED()){
          welcomeScreen();
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
        }
      break;
    }
  }
  if (wakeup_reason != ESP_SLEEP_WAKEUP_EXT0){
    if (String(IP) != "no server" && AHT10_alive && VL53L0X_alive){
      time_sleep_left = Update(serverName, sensorName, sensorLocation);
    } else {
      time_sleep_left = TIME_TO_UPDATE_IN_SEC;
    }
  }
  
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
  esp_sleep_enable_timer_wakeup(time_sleep_left*1000000-8000000); 
  touchAttachInterrupt(TOUCH, TSR, threshold_touch);
  esp_sleep_enable_touchpad_wakeup();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 1);
  digitalWrite(ERROR, LOW);
  Serial.println("Sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}
void loop(){}
