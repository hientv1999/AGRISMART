#include "OLED.hpp"
#include "WiFi_functions.hpp"
#include <AHT10.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "error.hpp"
extern int ERROR;
AHT10 myAHT10(AHT10_ADDRESS_0X38);
bool turnOnTempHum(){
   unsigned long start_time = millis();
   while (myAHT10.begin() != true && millis() - start_time <= 5000)
   {
      // TempHum_error();
   }
   if (myAHT10.begin() != true){
      printlnClearOLED(processText("Fail to initialize AHT10 sensor").c_str(), WHITE, 1);
      Serial.println("AHT10 failed");
      return false;
   }
   // ledcDetachPin(ERROR);
   Serial.println("AHT10 OK");
   return true;
}
String getTemperature(){
   String temperature = String(myAHT10.readTemperature(), 2);
   return temperature;
}
String getHumidity(){
   String humidity = String(myAHT10.readHumidity(), 2);
   return humidity;
}
