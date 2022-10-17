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
bool touchDetect;
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

void IRAM_ATTR cycleOLED(){
  digitalWrite(HAPTIC, HIGH);
  digitalWrite(HAPTIC, LOW);
  touchDetect = true;
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
  bool ADS1115_alive = false;
  float USB_PLUG_volt;
  if (FirstSetup() == 0){                 // first time setup
    // turn on OLED, display welcome screen, turn on sensor, turn on BLE
    turnOnOLED();
    turnOnBLE("Unnamed Agrismart");
    welcomeScreen();
    AHT10_alive = turnOnTempHum();
    VL53L0X_alive = turnOnTOF();
    ADS1115_alive = turnOnADC();
    if (setupOption(true)){
      //fast setup
      while (!fastSetup(ssid, pass, IP, sensorName, sensorLocation, gmtOffset_sec)){
        delay(500);
        if (!setupOption(false)){//if switch to manual setup
          // show instructions for manual setup
          instructionToLightBlue();
          // ask for SensorName -> SSID -> Pass -> IP -> Sensor Location 
          strcpy(sensorName, askForSensorName().c_str());
          saveSensorName(sensorName, strlen(sensorName));
          strcpy(sensorLocation, askForSensorLocation().c_str());
          saveSensorLocation(sensorLocation, strlen(sensorLocation));
          bool WiFi_connection = false;
          while (!WiFi_connection){
            strcpy(ssid, askForSSID().c_str());
            strcpy(pass, askForPass().c_str());
            saveSSID(ssid, strlen(ssid));
            savePassword(pass, strlen(pass));
            WiFi_connection = turnOnWiFi(sensorName, true); 
          }
          strcpy(IP, askForIP().c_str());
          gmtOffset_sec = askForOffset();
        }
      }
    } else {
      // show instructions for manual setup
      instructionToLightBlue();
      // ask for SensorName -> SSID -> Pass -> IP -> Sensor Location 
      strcpy(sensorName, askForSensorName().c_str());
      saveSensorName(sensorName, strlen(sensorName));
      strcpy(sensorLocation, askForSensorLocation().c_str());
      saveSensorLocation(sensorLocation, strlen(sensorLocation));
      bool WiFi_connection = false;
      while (!WiFi_connection){
        strcpy(ssid, askForSSID().c_str());
        strcpy(pass, askForPass().c_str());
        saveSSID(ssid, strlen(ssid));
        savePassword(pass, strlen(pass));
        WiFi_connection = turnOnWiFi(sensorName, true); 
      }
      strcpy(IP, askForIP().c_str());
      gmtOffset_sec = askForOffset();
    }
    
    saveIP(IP, strlen(IP));
    saveOffset(gmtOffset_sec);
    // force device to update to server to obtain local timezone
    lastUpdate = getTime() - TIME_TO_UPDATE_IN_SEC;
    serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
    turnOffBLE();
    printlnClearOLED("Setup done", WHITE, 1);
    unsigned int start_finish = millis();
    while (millis() - start_finish < 2500);
    turnOffOLED();
    FinishSetup();
  } else {
    switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_TOUCHPAD : //touch pad
      {
        Serial.println("wake up due to touch pad");
        digitalWrite(HAPTIC, HIGH);
        delay(100);
        digitalWrite(HAPTIC, LOW);
        // read SOLAR_VOLT < 1V then pull NIGHT_LIGHT high, at the end pull NIGHT_LIGHT low
        if (turnOnOLED()){
          displayTurnOnAnimation();
          selfTestEEPROM();
          strcpy(sensorName, retrieveSensorName().c_str());
          strcpy(IP, retrieveIP(sensorName).c_str());
          turnOnWiFi(sensorName, true);
          AHT10_alive = turnOnTempHum();
          VL53L0X_alive = turnOnTOF();
          ADS1115_alive = turnOnADC();

          if (ADS1115_alive){
            if (solarVoltage() - 1 < 0){
              digitalWrite(NIGHT_LIGHT, HIGH);
            }
          }
          strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
          gmtOffset_sec = retrieveOffset(sensorName);
          configTime(gmtOffset_sec, 0, "pool.ntp.org");
          serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
          Serial.println("Prepare to display");
          touchAttachInterrupt(TOUCH, cycleOLED , threshold_touch);
          touchDetect = false;
          togglePlug = false;
          unsigned long lastPressOLED = millis();
          unsigned long lastTouch = millis();
          bool toggleScreen = false;
          while (millis() - lastPressOLED <= 15000){ //cycle through display mode
            // USB screen if plug/unplug
            USB_PLUG_volt = ReadVoltageAnalogPin(USB_PLUG);
            if (USB_PLUG_volt >= 2 && charging_plug == true){  // unplug
              charging_plug = false;
              digitalWrite(DISABLE_CHARGE, LOW);
              Serial.println("No charging");
              displayPlugInUSB_C(false);
            }
            if (USB_PLUG_volt < 2 && charging_plug == false){ // plug
              charging_plug = true;
              float CC_volt = ReadVoltageAnalogPin(CC);
              Serial.print("CC volt: ");
              Serial.println(CC_volt, 2);
              if (CC_volt - 0.33 > 0 && CC_volt - 0.86 < 0){  // USB cable not supply enough current
                digitalWrite(DISABLE_CHARGE, HIGH);
                displayWarningPlug();
                Serial.println("Not enough current");
              } else if (CC_volt - 0.86 > 0){                 // enough current
                digitalWrite(DISABLE_CHARGE, LOW);
                Serial.println("Start charging");
                displayPlugInUSB_C(true);
              } else {                                        // unplug cable
                digitalWrite(DISABLE_CHARGE, LOW);
                Serial.println("No charging");
                displayPlugInUSB_C(true);
              }
            }
            // normal screen
            switch (tap_num){
              case 0: //everything
                displayOverview(AHT10_alive, VL53L0X_alive, ADS1115_alive);
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
                displayBatteryLevel(ADS1115_alive); 
              break;

              case 5: // wifi signal
                displayWiFi(); 
              break;

              case 6: // setup water level low
                lastPressOLED = millis();
                printlnClearOLED(processText("Tap to set current level as BOTTOM level. Wait to back to main screen").c_str(), WHITE, 1);
                if (millis() - lastTouch >= 10000){
                  tap_num = 3;
                } else {
                  if (touchDetect){
                    if (millis() - lastTouch >= 3000){
                      saveBottomWaterLevel();
                      tap_num = 7;
                      lastTouch = millis();
                    }
                    touchDetect = false;
                  }
                  
                }
              break;
              
              case 7: // setup water level high
                lastPressOLED = millis();
                printlnClearOLED(processText("Tap to set current level as TOP level. Wait to back to main screen").c_str(), WHITE, 1);
                if (millis() - lastTouch >= 10000){
                  tap_num = 3;
                } else {
                  if (touchDetect){
                    if (millis() - lastTouch >= 3000){
                      saveTopWaterLevel();
                      tap_num = 3;
                      lastTouch = millis();
                    }
                    touchDetect = false;
                  }
                }
              break;

              default:  // never get here, error occurs
                printlnClearOLED(processText("Unexpected eror. Device will restart in 10 seconds").c_str(), WHITE, 1);
                delay(10000);
                ESP.restart();  
            }
            if (tap_num <6){
              if (touchDetect){
                toggleScreen = true;
                if (millis() - lastTouch >= 500){
                  if (millis() - lastTouch < 750 && tap_num == 3){  // long press on water level screen
                    toggleScreen = false;
                    tap_num = 6;        
                  }
                  lastTouch = millis();
                } 
                touchDetect = false;
              } else {
                if (millis() - lastTouch >=750 && toggleScreen){
                  tap_num = (tap_num+1)%6;
                  lastPressOLED = millis();
                  toggleScreen = false;
                }
              }
            }
            
          }    
          Serial.println("End of screen");
          displayTurnOffAnimation();
          turnOffOLED();
          digitalWrite(NIGHT_LIGHT, LOW);
        }
      }
    break;
    
    case ESP_SLEEP_WAKEUP_TIMER : //timer
      Serial.println("wake up due to timer");
      strcpy(sensorName, retrieveSensorName().c_str());
      turnOnWiFi(sensorName, false);
      //turn sensors on
      AHT10_alive = turnOnTempHum(false);
      VL53L0X_alive = turnOnTOF(false);
      strcpy(IP, retrieveIP(sensorName).c_str());
      strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
      serverName = "http://" + String(IP) + "/user/gardening/Agrismart/post_data.php";
    break;
    
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("USB Type-C state changes");
      digitalWrite(HAPTIC, HIGH);
      delay(100);
      digitalWrite(HAPTIC, LOW);
      if (turnOnOLED()){
        // USB screen if plug/unplug
        USB_PLUG_volt = ReadVoltageAnalogPin(USB_PLUG);
        if (USB_PLUG_volt >= 2 && charging_plug == true){  // unplug
          charging_plug = false;
          digitalWrite(DISABLE_CHARGE, LOW);
          Serial.println("No charging");
          displayPlugInUSB_C(false);
        }
        if (USB_PLUG_volt < 2 && charging_plug == false){ // plug
          charging_plug = true;
          float CC_volt = ReadVoltageAnalogPin(CC);
          Serial.print("CC volt: ");
          Serial.println(CC_volt, 2);
          if (CC_volt - 0.33 > 0 && CC_volt - 0.86 < 0){  // USB cable not supply enough current
            digitalWrite(DISABLE_CHARGE, HIGH);
            displayWarningPlug();
            Serial.println("Not enough current");
          } else if (CC_volt - 0.86 > 0){                 // enough current
            digitalWrite(DISABLE_CHARGE, LOW);
            Serial.println("Start charging");
            displayPlugInUSB_C(true);
          } else {                                        // unplug cable
            digitalWrite(DISABLE_CHARGE, LOW);
            Serial.println("No charging");
            displayPlugInUSB_C(true);
          }
        }
        turnOffOLED();
      }
    break;

    default:  // wakeup by reset
        if (turnOnOLED()){
          welcomeScreen();
          strcpy(sensorName, retrieveSensorName().c_str());
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
              if (EEPROM.read(3) != 0){
                factoryReset(false);
              } else {
                strcpy(IP, retrieveIP(sensorName).c_str());
                strcpy(sensorLocation, retrieveSensorLocation(sensorName).c_str());
                serverName = "http://" + String(IP) + "/user/gardening/Agrismart/deleteTable.php";
                if (turnOnWiFi(sensorName, true) && deleteTable(serverName, sensorName, sensorLocation)){
                  factoryReset(false);
                } else {
                  if (ManualFactoryReset()){
                    factoryReset(true);
                  }
                }
              }
            }
          }
          
          time_sleep_left = TIME_TO_UPDATE_IN_SEC;
        }
      break;
    }
  }
  if (!charging_plug){
    if (String(IP) != "no server" && AHT10_alive && VL53L0X_alive && ADS1115_alive){
      time_sleep_left = Update(serverName, sensorName, sensorLocation);
    } else {
      time_sleep_left = TIME_TO_UPDATE_IN_SEC;
    }
    esp_sleep_enable_timer_wakeup(time_sleep_left*1000000-8000000); 
    touchAttachInterrupt(TOUCH, TSR, threshold_touch);
    esp_sleep_enable_touchpad_wakeup();
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);
    
    Serial.println("Sleep now");
    Serial.flush();
  } else {
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_timer_wakeup(1000000*36000);    // sleep for 10 hours (should be more than enough to fully charge battery)
    touchAttachInterrupt(TOUCH, TSR, threshold_touch);
    esp_sleep_enable_touchpad_wakeup();
    Serial.println("Charge sleep now");
    Serial.flush();
  }
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
  digitalWrite(ERROR, LOW);
  esp_deep_sleep_start();
}
void loop(){}
