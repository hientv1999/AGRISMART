#include "OLED.hpp"
#include "WiFi_functions.hpp"
#include <AHT10.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "error.hpp"
extern int ERROR;
AHT10 myAHT10(AHT10_ADDRESS_0X38);
bool turnOnTempHum(bool display_text){
   unsigned long start_time = millis();
   while (myAHT10.begin() != true && millis() - start_time <= 3000)
   {
      // TempHum_error();
      if (display_text){
         printlnClearOLED(processText("Temperature/humidity sensor failed").c_str(), WHITE, 1);
         delay(500);
      }
   }
   if (myAHT10.begin() != true){
      Serial.println("AHT10 failed");
      return false;
   }
   // ledcDetachPin(ERROR);
   Serial.println("AHT10 OK");
   return true;
}
String getTemperature(bool AHT10_alive){
   if (!AHT10_alive){
      return "???";
   } else {
      String temperature = String(myAHT10.readTemperature(), 2);
      return temperature;
   }
   
}
String getHumidity(bool AHT10_alive){
   if (!AHT10_alive){
      return "???";
   } else {
      String humidity = String(myAHT10.readHumidity(), 2);
      return humidity;
   }
   
}
