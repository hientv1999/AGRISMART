#include "OLED.hpp"
#include "WiFi_functions.hpp"
#include <AHT10.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
extern int ERROR;
AHT10 myAHT10(AHT10_ADDRESS_0X38);
bool turnOnTempHum(bool display_text){
   unsigned long start_time = millis();
   while (!myAHT10.begin() && millis() - start_time <= 3000)
   {
      // TempHum_error();
      if (display_text){
         printlnClearOLED(processText("Temperature/humidity sensor failed").c_str(), WHITE, 1);
         delay(500);
      }
   }
   if (!myAHT10.begin()){
      Serial.println("AHT10 failed");
      return false;
   }
   Serial.println("AHT10 OK");
   return true;
}
String getTemperature(bool AHT10_alive){
   if (!AHT10_alive){
      return "?? ";
   } else {
      float sum = 0;
      int i = 0;
      float value;
      while (i<3){
         value = myAHT10.readTemperature();
         if (value < 55 && value > -50){
            sum += value;
            i ++;
         }
      }
      String temperature = String(sum/3, 2);
      return temperature;
   }
}
String getHumidity(bool AHT10_alive){
   if (!AHT10_alive){
      return "?? ";
   } else {
      float sum = 0;
      int i = 0;
      float value;
      while (i<3){
         value = myAHT10.readHumidity();
         if (value <= 100 && value >= 0){
            sum += value;
            i ++;
         }
      }
      String humidity = String(sum/3, 2);
      return humidity;
   }
   
}
