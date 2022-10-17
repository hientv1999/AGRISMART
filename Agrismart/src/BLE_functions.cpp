#include <NimBLEDevice.h>
#include <EEPROM.h>
#include "EEPROM_functions.hpp"
#include "OLED.hpp"
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SERVICE_UUID2        "44f2eb7c-4077-444c-a935-ef50b26fe741"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "42eab6ec-829e-4d99-b48f-81481d077948"
NimBLECharacteristic *pCharacteristic;
NimBLECharacteristic *pCharacteristic2;
extern int CYCLE_OLED;
extern int TOUCH;
extern uint16_t threshold_touch;
extern int getTouchValue(uint16_t pin);
void turnOnBLE(std::string sensorName){
    BLEDevice::init(sensorName);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    NimBLEService *pService2 = pServer->createService(SERVICE_UUID2);
    pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE  );
    pCharacteristic2 = pService2->createCharacteristic(CHARACTERISTIC_UUID2, NIMBLE_PROPERTY::READ );
    
    pService->start();
    pService2->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    NimBLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    NimBLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}
bool fastSetup(char (&wifiName)[32], char (&password)[63], char (&ip)[15], char (&name)[32], char (&location)[32], long &gmtOffset_sec){
    printlnClearOLED(processText("Access website to fast setup Agrismart Tap to abort").c_str(), WHITE, 1);
    std::string OTP = "";
    std::string final_OTP = "";
    while (OTP != "validate"){  //wait for "validate" message
        //get OTP
        final_OTP = OTP;
        OTP  = pCharacteristic -> getValue();
        while (OTP.length() <=1){
            OTP  = pCharacteristic -> getValue();
            if (getTouchValue(TOUCH) < threshold_touch){
                return false;
            }
        }
        printlnClearOLED(" ", WHITE, 2);
        printlnOLED(("   " + OTP).c_str(), WHITE, 2);
        pCharacteristic -> setValue("");
        
    }
    Serial.println("Received OTP and 'validate' message");
    pCharacteristic2 -> setValue(final_OTP);
    //get SSID
    std::string ssid  = pCharacteristic -> getValue();
    printClearOLED("Loading", WHITE, 1);
    while (ssid.length() <=1){
      ssid  = pCharacteristic -> getValue();
    }
    Serial.println("Received SSID");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    pCharacteristic2 -> setValue("");
    //get Pass
    std::string pass  = pCharacteristic -> getValue();
    while (pass.length() <=1){
      pass  = pCharacteristic -> getValue();
    }
    Serial.println("Received Pass");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    //get Location
    std::string loc  = pCharacteristic -> getValue();
    while (loc.length() <=1){
      loc  = pCharacteristic -> getValue();
    }
    Serial.println("Received location");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    //get sensor name
    std::string sensorName  = pCharacteristic -> getValue();
    while (sensorName.length() <=1){
      sensorName  = pCharacteristic -> getValue();
    }
    Serial.println("Received sensor name");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    //get IP
    std::string IP  = pCharacteristic -> getValue();
    while (IP.length() <=1){
      IP  = pCharacteristic -> getValue();
    }
    Serial.println("Received IP");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    //get Offset
    std::string data  = pCharacteristic -> getValue();
    while (data.length() <=1){
      data  = pCharacteristic -> getValue();
    }
    Serial.println("Received offset");
    printOLED(".", WHITE, 1);
    pCharacteristic -> setValue("");
    //check WiFi connection
    WiFi.disconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    std::string return_msg = "Setup success";
    int i = 0;
    unsigned int start = millis();
    while (WiFi.status() != WL_CONNECTED){
        printOLED(".", WHITE, 1);
        delay(500);
        i++;
        if (i >= 3){
            printClearOLED("Connecting", WHITE, 1);
            i = 0;
        }
        if (millis() - start >= 10000){
            printClearOLED(processText("Fail to connect to WiFi, please move closer to the WiFi router").c_str(), WHITE, 1);
            delay(5000);
            return_msg = "Setup fail";
            break;
        }
    }
    // return message about WiFi connection
    pCharacteristic2 -> setValue(return_msg);
    if (return_msg == "Setup success"){
        printClearOLED("Saving", WHITE, 1);
        strcpy(wifiName, ssid.c_str());
        saveSSID(wifiName, strlen(wifiName));
        printOLED(".", WHITE, 1);
        strcpy(password, pass.c_str());
        savePassword(password, strlen(password));
        printOLED(".", WHITE, 1);
        strcpy(location, loc.c_str());
        saveSensorLocation(location, strlen(location));
        printOLED(".", WHITE, 1);
        strcpy(name, sensorName.c_str());
        saveSensorName(name, strlen(name));
        printOLED(".", WHITE, 1);
        strcpy(ip, IP.c_str());
        printOLED(".", WHITE, 1);
        const char* offset = data.c_str();
        if (offset[0] == '-'){
            gmtOffset_sec = atoi(&offset[1]) * -1;
        } else {
            gmtOffset_sec = atoi(offset);
        }
        printOLED(".", WHITE, 1);
    } else {
        return false;
    }
    return true;
}

std::string askForSSID(){
    printlnClearOLED("Searching for WiFi...", WHITE, 1);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int available_wifi = WiFi.scanNetworks();
    pCharacteristic -> setValue("");
    int order = 1;
    bool correct_value = false;
    std::string confirmation;
    while (available_wifi == 0){
        printlnClearOLED(processText("No available WiFi in this area. Try another place").c_str(), WHITE, 1);
    } 
    printlnClearOLED(("1 - " + WiFi.SSID(order-1)).c_str(), WHITE, 1);
    printOLED(processText("Is this your WiFi? Type Yes to choose. Tap to next").c_str(), WHITE, 1);
    while (!correct_value){
        confirmation  = pCharacteristic -> getValue();
        while (confirmation.length() <= 1){
            if (getTouchValue(TOUCH) < threshold_touch){
                order = (order+1)%available_wifi;
                if (order == 0){
                    order ++;
                }
                printlnClearOLED((String(order) + " - " + WiFi.SSID(order-1)).c_str(), WHITE, 1);
                printOLED(processText("Is this your WiFi? Type Yes to choose. Tap to next").c_str(), WHITE, 1);
            }
            confirmation = pCharacteristic -> getValue();
            delay(200);
        }
        pCharacteristic -> setValue("");
        if (confirmation != "Yes" && confirmation.length() > 1){
                printlnClearOLED(processText("Please type Yes only").c_str(), WHITE, 1);
                printlnOLED("Try again now", WHITE, 1);      
        } else {
            correct_value = true;
        }
    }
    return WiFi.SSID(order-1).c_str();
}
std::string askForPass(){
   pCharacteristic ->setValue("");
   std::string Pass;
   while (Pass.length() <= 1){
        printClearOLED(processText("Type your WiFi password").c_str(), WHITE, 1);
        Pass = pCharacteristic -> getValue();
    }
   return Pass;
}

std::string askForIP(){
   pCharacteristic ->setValue("");
   std::string IP;
    IP = pCharacteristic -> getValue();
    while (IP.length() <= 1){
        printClearOLED(processText("Type your IP Server. Type \"No server\" if you don't have the server").c_str(), WHITE, 1);
        IP = pCharacteristic -> getValue();
        std::for_each(IP.begin(), IP.end(), [](char & c){c = ::tolower(c);});
    }
    while (getTouchValue(TOUCH) > threshold_touch){
        String text[2] = {IP.c_str(), "Tap to confirm"};
        printArrayOLED(text, 2, WHITE, 0, 0, 1);
        if (pCharacteristic -> getValue().length() > 1){
            IP = pCharacteristic -> getValue();
            pCharacteristic -> setValue("");
        }
    }
   return IP;
}
std::string askForSensorName(){
   pCharacteristic ->setValue("");
   std::string SensorName;
   SensorName = pCharacteristic -> getValue();
    while (SensorName.length() <= 1){
        printClearOLED(processText("Name your device (2-32 characters)").c_str(), WHITE, 1);
        SensorName = pCharacteristic -> getValue();
    }
    while (getTouchValue(TOUCH) > threshold_touch){
        if (SensorName.length() > 1){
            String text[2] = {SensorName.c_str(), "Tap to confirm"};
            printArrayOLED(text, 2, WHITE, 0, 0, 1);
            pCharacteristic ->setValue("");
        }
    }
   return SensorName;
}

std::string askForSensorLocation(){
    pCharacteristic ->setValue("");
    std::string SensorLocation;
    SensorLocation = pCharacteristic -> getValue();
        while (SensorLocation.length() <= 1){
            printClearOLED(processText("Type the location of this device (2-32 characters). Ex: garden, bedroom, ...").c_str(), WHITE, 1);
            SensorLocation = pCharacteristic -> getValue();
        }
        while (getTouchValue(TOUCH) > threshold_touch){
            if (SensorLocation.length() > 1){
                String text[2] = {SensorLocation.c_str(), "Tap to confirm"};
                printArrayOLED(text, 2, WHITE, 0, 0, 1);
                pCharacteristic -> setValue("");
            }
        }
    return SensorLocation;
}

long askForOffset(){
    pCharacteristic ->setValue("");
    char offset_string[6] = {};
    char* offset = offset_string;
    std::string tz = pCharacteristic -> getValue();
    while (tz.length() <= 1){
        printClearOLED(processText("Type the timezone offset in second (2-32 characters). Ex: -28800, -7200...").c_str(), WHITE, 1);
        tz = pCharacteristic -> getValue();
    }
    while (getTouchValue(TOUCH) > threshold_touch){
        String text[2] = {tz.c_str(), "Tap to confirm"};
        printArrayOLED(text, 2, WHITE, 0, 0, 1);
        if (pCharacteristic -> getValue().length() > 1){
            tz = pCharacteristic -> getValue();
            pCharacteristic -> setValue("");
        }
    }
    strcpy(offset, tz.c_str());
    if (offset[0] == '-'){
        return atoi(++offset) * -1;
    } else {
        return atoi(offset);
    }
    
}

void turnOffBLE(){
   NimBLEDevice::stopAdvertising();
}
