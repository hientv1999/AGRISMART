#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AHT10.h>
#include <WiFi.h>
#include "time.h"
#include "TempHum.hpp"
#include "watering.hpp"
#include "battery.hpp"
#include "error.hpp"
extern AHT10 myAHT10;
extern unsigned int NUM_OF_DATATYPE;
extern int BATT_LEVEL;
extern int ERROR;
extern int getTouchValue(uint16_t pin);
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define NUMDROPLETS 15
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
extern int BUTTON_OLED;
extern int TOUCH;
extern uint16_t threshold_touch;
extern RTC_DATA_ATTR bool charging_plug;

std::string processText(const char* text){
    std::string modified_text = text;
    for (int i=1; i<4; i++){
        if (modified_text.length() > 21*i-1){                    //string longer than 1 line
            if (modified_text[21*i-1]  != ' '){  //last character of current line isn't a space
                if (modified_text[21*i] != ' '){//first character of next line isn't a space
                    size_t index = 21*i-2; //find the nearest white space
                    while (modified_text[index] != ' '){
                        index --;
                    }
                    while (modified_text[21*i-1] != ' '){
                        modified_text.insert(index,1, ' ');
                    }
                } else {
                    modified_text.erase(21*i,1);
                    // erase this space
                }
            }
        }
    }
    return modified_text;
}

void printArrayOLED(const String text[], const unsigned int sizeArray, const uint16_t color, const int16_t x_cursor, const int16_t y_cursor, const uint8_t size){
   display.clearDisplay();
   display.setTextColor(color);
   display.setCursor(x_cursor, y_cursor);
   display.setTextSize(size);
   for( unsigned int i = 0; i < sizeArray; i = i + 1 ){
       display.println(text[i]);
   }
   display.display();
}
void printClearOLED(const char* text, const uint16_t color, const uint8_t size){
   display.clearDisplay();
   display.setTextColor(color);
   display.setCursor(0, 0);
   display.setTextSize(size);
   display.print(text);
   display.display();
}
void printlnClearOLED(const char* text, const uint16_t color, const uint8_t size){
   display.clearDisplay();
   display.setTextColor(color);
   display.setCursor(0, 0);
   display.setTextSize(size);
   display.println(text);
   display.display();
}
void printOLED(const char* text, const uint16_t color, const uint8_t size){
   display.setTextColor(color);
   display.setTextSize(size);
   display.print(text);
   display.display();
}
void printlnOLED(const char* text, const uint16_t color, const uint8_t size){
   display.setTextColor(color);
   display.setTextSize(size);
   display.println(text);
   display.display();
}

bool turnOnOLED(){
    pinMode(BUTTON_OLED, OUTPUT);
    digitalWrite(BUTTON_OLED, HIGH);
    delay(100);
    unsigned int origin = millis();
    bool OLED_alive = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    while(!OLED_alive && millis()- origin <= 10000) { // Address 0x3D for 128x64
        OLED_error();
        delay(1000);
        OLED_alive = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    }
    ledcDetachPin(ERROR);
    if (!OLED_alive){
        Serial.println("SSD1306 failed");
        return false;
    } else {
        Serial.println("OLED OK");
        return true;
    }
}

void turnOffOLED(){
    display.clearDisplay();
    display.display();
    digitalWrite(BUTTON_OLED, LOW);
}

bool setupOption(bool firstTime){
    if (firstTime){
        printlnClearOLED(processText("Welcome to Agrismart").c_str(), WHITE, 1);
        delay(3000);
        printlnOLED(processText("Tap for Manual Setup").c_str(), WHITE, 1);
        printlnOLED(processText("Wait for Auto Setup").c_str(), WHITE, 1);
    } else {
        printlnClearOLED(processText("Tap for Manual Setup").c_str(), WHITE, 1);
        printlnOLED(processText("Wait for Auto Setup").c_str(), WHITE, 1);
    }
    uint64_t start = millis();
    int t = 0;
    while (getTouchValue(TOUCH) > threshold_touch && t < 21){
        if (millis() - start >= 250){
            printOLED(".", WHITE, 1);
            start = millis();
            t++;
        }
    }
    if (t==21){
        return true;//return true means auto setup
    } else {
        return false;//return false means manual setup
    }
}

void instructionToLightBlue(){  
    while (getTouchValue(TOUCH) > threshold_touch){       
        printClearOLED(processText("Install the LightBlue app from Appstore. Touch the bottom to proceed").c_str(), WHITE, 1);
    }
    delay (500); 
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("You must connect your smartphone to this device via Bluetooth for initial setup").c_str(), WHITE, 1);
    }
    delay(500);
    
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("Launch the app, go to Peripherals section at the bottom").c_str(), WHITE, 1);
    }
    delay (500);
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("Locate and select the device named  \"Unnamed Agristmart\" in the list").c_str(), WHITE, 1);
    }
    delay (500);
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("Choose the one having Properties: Write").c_str(), WHITE, 1);
    }
    delay (500);
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("Go to Hex at the top right corner. Choose UTF-8 String").c_str(), WHITE, 1);
    }
    delay (500);   
    while (getTouchValue(TOUCH) > threshold_touch){
        printClearOLED(processText("Choose \"Write new value\" to start setup").c_str(), WHITE, 1);
    }
    delay (500); 
}

void displayPlug(){
    display.clearDisplay();
    

}

void welcomeScreen(){
    display.clearDisplay();
    const PROGMEM unsigned char vine[] = { 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x03,0xff,0xfb,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf8
    ,0x00,0x3f,0x00,0x78,0x3f,0x80,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x07,0xf8
    ,0x01,0xe0,0x00,0x7c,0x00,0xf8,0x00,0x00,0x7f,0x00,0x00,0x00,0x00,0x00,0x1f,0xe0
    ,0x00,0x00,0x00,0xff,0x00,0x0f,0x00,0x00,0xfe,0x00,0x00,0x1c,0x00,0x00,0x3f,0xc0
    ,0x00,0x00,0x00,0x7f,0x00,0x00,0xf0,0x01,0xfe,0x00,0x00,0x7c,0x00,0x03,0xfe,0x00
    ,0x00,0x00,0x00,0x7f,0x80,0x00,0xff,0x03,0xfc,0x00,0x00,0xfc,0x00,0x1f,0xc0,0x00
    ,0x00,0x00,0x00,0x3f,0xc0,0x01,0xf9,0xe3,0xf0,0x00,0x01,0xfc,0x00,0x78,0x00,0x00
    ,0x00,0x00,0x00,0x1f,0xc0,0x01,0xfc,0x7f,0x80,0x00,0x03,0xf8,0x07,0xc0,0x00,0x00
    ,0x00,0x00,0x00,0x07,0xc0,0x01,0xfe,0x07,0x80,0x00,0x03,0xf0,0x3f,0xc0,0x00,0x00
    ,0x00,0x00,0x00,0x01,0xc0,0x00,0xfe,0x00,0x7e,0x00,0x01,0xe3,0xe7,0xf8,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,0x07,0xf0,0x0b,0xef,0x03,0xfe,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x0f,0xbe,0x00,0x01,0xff,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00,0x00,0x0f,0x80,0x00,0x00,0x3f,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x0f,0xc0,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xe0,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xf0,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xf0,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xf8,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    const PROGMEM unsigned char vine_empty[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x03,0xff,0xfb,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf8
    ,0x00,0x3f,0x00,0x00,0x3f,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xf0
    ,0x01,0xe0,0x00,0x00,0x00,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x80
    ,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xf0,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xc0,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xe0,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x07,0xc0,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xc0,0x00,0x00,0x00,0x3f,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x03,0xe0,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xf0,0x0b,0xff,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xfe,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    
    int droplets[NUMDROPLETS][2];
    for (int i=0; i<NUMDROPLETS; i++){
        droplets[i][0] = random(0, 128);
        droplets[i][1] = 0;
    }
    unsigned long current_time = millis();
    int slider = 0;
    int drop = NUMDROPLETS-1;
    while (millis() - current_time < 7000){
        display.clearDisplay();
        // draw the animated leaves
        display.drawBitmap(0, 0, vine, 128, 32, WHITE);
        display.fillRect(slider-20, 0, 148-slider, 32, BLACK);
        // draw the empty vine
        display.drawBitmap(0, 0, vine_empty, 128, 32, WHITE);
        display.fillRect(slider, 0 , 128-slider, 32, BLACK);
        if (millis() - current_time > 1500){
            slider += 4 ; 
        }
        // draw the rain
        if (millis() - current_time >500){
            for (int i=0; i< NUMDROPLETS-drop; i++){
                display.writeFastVLine(droplets[i][0], droplets[i][1], 4, WHITE);
                droplets[i][1] += 8;
                if (droplets[i][1] > 31 && slider <= 140){
                    droplets[i][1] = random(0,28);
                }
            }
            //gradient rain
            if (drop >0){
                drop--;
            }
        }
        display.display();
        delay(100);
    }
}

void displayPlugInUSB_C(bool plug){
    const PROGMEM unsigned char usb_c_cable_plugin[] = {
        0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x3f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    unsigned long start_time = millis();
    int i=0;
    if (plug){
        i=128;
    }
    while (millis() - start_time <= 1250){
        display.clearDisplay();
        // draw socket
        display.drawRoundRect(19, 4, 11, 27, 6, WHITE);
        display.drawRoundRect(20, 5, 9, 25, 5, WHITE);  
        display.drawFastVLine(24, 8, 19, WHITE);
        // black box
        display.fillRect(i+24, 8 , 40, 19, BLACK);
        // draw cable
        display.drawBitmap(i+24, 5, usb_c_cable_plugin, 192, 26, WHITE);
        if (i>14){
            display.drawRoundRect(i+10, 9, 16, 18, 2, WHITE);  
            display.drawRoundRect(i+9, 8, 18, 20, 3, WHITE); 
        } else {
            display.drawRoundRect(24, 9, i+1, 18, 2, WHITE);  
            display.drawRoundRect(24, 8, i+3, 20, 3, WHITE); 
        }
        display.display();
        if (plug){
            if (i > 40){
                i-=4;
            } else if (i>20){
                i-=3;
            } else if (i>7){
                i-=2;
            } else if (i>0){
                i-=1;
            }
        } else {
            if (i<7){
                i+=1;
            } else if (i<20){
                i+=2;
            } else if (i<40){
                i+=3;
            } else if (i<128){
                i+=4;
            }
        }
    }   
}

void displayWarningPlug(){
    printClearOLED(processText("USB Type-C not enough power. Please change cable and/or block.").c_str(), WHITE, 1);
}

void displayTurnOnAnimation(){
    // point to line
    for (int i=63; i>0; i-=8){
        delay(10);
        display.clearDisplay();
        display.drawRoundRect(i, 15, (64-i)*2, 3, 3, WHITE);
        display.display();
        
    }
    // line to rectangle
    for (int i=0; i<=16; i+=2){
        delay(10);
        display.clearDisplay();
        display.drawRoundRect(-2, 15-i, 132, 3+i*2, 3, WHITE);
        display.display();
    }
}

void displayTurnOffAnimation(){
    // line to rectangle
    for (int i=16; i>=0; i-=2){
        delay(10);
        display.clearDisplay();
        display.drawRoundRect(-2, 15-i, 132, 3+i*2, 3, WHITE);
        display.display();
    }
    // point to line
    for (int i=-1; i<=63; i+=8){
        delay(10);
        display.clearDisplay();
        display.drawRoundRect(i, 15, (64-i)*2, 3, 3, WHITE);
        display.display();
    }
}

void displayOverview(bool AHT10_alive, bool VL53L0X_alive, bool ADS1115_alive){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    //Battery level
    display.drawFastHLine(1, 1, 12, WHITE);
    display.drawFastVLine(13, 1, 2, WHITE);
    display.drawFastHLine(13, 3, 2, WHITE);
    display.drawFastVLine(15, 3, 5, WHITE);
    display.drawFastHLine(13, 7, 2, WHITE);
    display.drawFastVLine(13, 8, 2, WHITE);
    display.drawFastHLine(1, 9, 12, WHITE);
    display.drawFastVLine(1, 1, 8, WHITE);
    display.fillRect(13,3,2,4,WHITE);
    if (ADS1115_alive){
        unsigned int batteryLevel = getBatteryLevel();
        if (batteryCharging()){
            display.drawLine(3, 3, 9, 7, WHITE);
            display.drawLine(7, 3, 9, 7, WHITE);
            display.drawLine(7, 3, 12, 7, WHITE);
        } else {
            display.fillRect(1,1,12*batteryLevel/100,8, WHITE);
        }
        display.setCursor(20, 2);
        display.print(String(batteryLevel) + "%");
    } else {
        display.setCursor(20, 2);
        display.print("?%");
    }
    //Time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)){
        display.setCursor(40, 2);
        if (timeinfo.tm_hour < 10){
            display.print("0");
        }
        display.print(String(timeinfo.tm_hour) + ":");
        if (timeinfo.tm_min < 10){
            display.print("0");
        }
        display.print(String(timeinfo.tm_min) + ":");
        if (timeinfo.tm_sec < 10){
            display.print("0");
        }
        display.print(String(timeinfo.tm_sec));
    } 
    // USB Plug
    if (charging_plug){ 
        display.drawRoundRect(94, 3, 18, 7, 3, WHITE);
        display.drawFastHLine(97, 6, 12, WHITE);
    }
    // WiFi signal
    int signal_strengh = WiFi.RSSI();
    display.drawFastVLine(116, 7, 3, WHITE);
    if (signal_strengh > -75){
        display.drawFastVLine(118, 4, 6, WHITE);
    }
    if (signal_strengh > -55){
        display.drawFastVLine(120, 1, 9, WHITE);
    }   
    // Information
    display.drawFastHLine(0, 12, 128, WHITE);
    display.drawFastHLine(0, 22, 128, WHITE);
    display.drawFastHLine(0, 31, 128, WHITE);
    display.drawFastVLine(43, 12, 20, WHITE);
    display.drawFastVLine(86, 12, 20, WHITE);
    display.drawFastVLine(0, 12, 20, WHITE);
    display.drawFastVLine(127, 12, 20, WHITE);
    display.setCursor(2, 14);
    display.print(getTemperature(AHT10_alive) + char(247) + "C");
    display.setCursor(2, 24);
    display.print(getHumidity(AHT10_alive) + "%");
    display.setCursor(46, 14);
    display.print(waterLevelPercentage(VL53L0X_alive) + "%");
    if (ADS1115_alive){
        float chargePower = chargingPower();
        if (chargingPower() > 0){
            display.setCursor(50, 24);
        } else {
            display.setCursor(46, 24);
        }
        display.print(String(chargePower, 2) + "W");
        display.setCursor(98, 14);
        display.print(String(soilMoisture(), 2) + "W");
        display.setCursor(98, 24);
        display.print(String(solarVoltage(), 2) + "V");
    } else {
        display.setCursor(50, 24);
        display.print("?? W");
        display.setCursor(98, 14);
        display.print("?? %");
        display.setCursor(98, 24);
        display.print("?? V");
    }
    display.display();
}

void displayTemperature(bool AHT10_alive){   // from -20C to 40C is proportionally display
    static PROGMEM const unsigned char Temperature[] = 
    {   0x01, 0xe0, 0x00, 0x03, 0xf8, 0x00, 0x06, 0x18, 0x00, 0x0c, 0x0d, 0x80, 0x0c, 0x0c, 0x00, 0x0c, 
	0x0c, 0x00, 0x0c, 0x0d, 0x80, 0x0c, 0x0c, 0x80, 0x0c, 0x0c, 0x00, 0x0c, 0x0d, 0x80, 0x0c, 0x0d, 
	0x80, 0x0c, 0x0c, 0x00, 0x0c, 0x0c, 0x00, 0x0c, 0x09, 0x80, 0x0c, 0x0c, 0x00, 0x18, 0x06, 0x00, 
	0x31, 0xe3, 0x00, 0x63, 0xf1, 0x80, 0x67, 0xf9, 0x80, 0x4f, 0xfc, 0xc0, 0xcf, 0xfc, 0xc0, 0xcf, 
	0xfc, 0xc0, 0xcf, 0xfc, 0xc0, 0x47, 0xf8, 0x80, 0x67, 0xf9, 0x80, 0x33, 0xf3, 0x00, 0x38, 0x03, 
	0x00, 0x1c, 0x0e, 0x00, 0x0f, 0xfc, 0x00, 0x01, 0xe0, 0x00    };
    display.clearDisplay();
    display.drawBitmap(5, 1, Temperature, 18, 30, 1);
    String text;
    if (AHT10_alive){
        float tempInC = myAHT10.readTemperature();
        if (tempInC > 40.0){
            display.fillRect(13, 5, 2, 12, SSD1306_INVERSE);
        } else if (tempInC <= 40.0 && tempInC >= -20.0){
            display.fillRect(13, 13 - int(12*(tempInC+20)/60) + 4, 2, int(12*(tempInC+20)/60), SSD1306_INVERSE);
        }
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(35, 16);
        text = String(myAHT10.readTemperature(), 2) + char(247) + 'C';
    } else {
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(35, 16);
        text = String("???") + char(247) + 'C';
    }
    display.print(text);
    display.display();
}

void displayHumidity(bool AHT10_alive){
    static PROGMEM const unsigned char Humidity[] = 
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 
	0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 
	0x07, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x80, 0x1f, 
	0xff, 0x00, 0x01, 0xc0, 0x1f, 0xff, 0x00, 0x01, 0xe0, 0x3f, 0xff, 0x80, 0x03, 0xf0, 0x3f, 0xff, 
	0x80, 0x07, 0xf8, 0x3f, 0xff, 0x80, 0x0f, 0xfc, 0x3f, 0xff, 0x80, 0x0f, 0xfc, 0x3f, 0xff, 0x80, 
	0x1f, 0xfe, 0x3f, 0xff, 0x80, 0x3f, 0xfe, 0x1f, 0xff, 0x00, 0x3f, 0xff, 0x1f, 0xff, 0x00, 0x3f, 
	0xff, 0x0f, 0xfe, 0x00, 0x7f, 0xff, 0x03, 0xf8, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xff, 
	0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 
	0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };
    display.clearDisplay();
    display.drawBitmap(7, 1, Humidity, 34, 30, 1);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(45, 16);
    String text;
    if (AHT10_alive){
        text = String(myAHT10.readHumidity(), 2) + '%';
    } else {
        text = "???%";
    }
    display.print(text);
    display.display();
}

void displayWaterLevel(bool VL53L0X_alive){
    // static PROGMEM const unsigned char Water[] = 
    // {   0x00, 0xc0, 0x00, 0x01, 0xe0, 0x00, 0x07, 0x18, 0x00, 0x1c, 0x0e, 0x00, 0x30, 0x03, 0x80, 0xff, 
    // 0xff, 0xc0, 0x60, 0x01, 0x80, 0x7f, 0xff, 0x80, 0x60, 0x01, 0x80, 0x60, 0xc0, 0x80, 0x61, 0xe0, 
    // 0x80, 0x63, 0x30, 0x80, 0x62, 0x10, 0x80, 0x63, 0x90, 0x80, 0x63, 0xb0, 0x80, 0x61, 0xe0, 0x80, 
    // 0x60, 0x00, 0x80, 0x7f, 0xff, 0x80, 0x60, 0x01, 0x80, 0xef, 0xfd, 0x80, 0x7b, 0xf7, 0x00, 0x18, 
    // 0x06, 0x00, 0x1e, 0x1e, 0x00, 0x13, 0x32, 0x00, 0x11, 0xe2, 0x00, 0x11, 0xe2, 0x00, 0x13, 0x32, 
    // 0x00, 0x1e, 0x1e, 0x00, 0x18, 0x06, 0x00, 0x10, 0x02, 0x00, 0x10, 0x02, 0x00    };
    display.clearDisplay();
    // display.drawBitmap(10, (display.height() - 31)/2, Water, 18, 31, 1);
    display.drawFastVLine(20, 2, 28, WHITE);
    display.drawFastHLine(14, 2, 6, WHITE);
    display.drawFastHLine(18, 9, 3, WHITE);
    display.drawFastHLine(14, 16, 6, WHITE);
    display.drawFastHLine(18, 23, 3, WHITE);
    display.drawFastHLine(14, 30, 10, WHITE);
    if (VL53L0X_alive){
        int level =waterLevelPercentage(true).toInt();
        if (level > 100){
            display.fillRect(22, 2, 10, 28, WHITE);
        } else if (level >0){
            display.fillRect(22, 30-level*28/100, 10, level*28/100, WHITE);
        }
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(60, 12);
        display.print(level);
    } else {
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(60, 12);
        display.print("???");
    }
    display.print("%");
    display.display();
    
    
}
void displayBatteryLevel(bool ADS1115_alive){
    static PROGMEM const unsigned char battery[] =
    { 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
	0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80 };
    static const unsigned char PROGMEM lightning[] =
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x30, 0x00, 0x1c, 0x38, 0x00, 0x07, 
	0xbe, 0x00, 0x03, 0xff, 0x80, 0x00, 0xf9, 0xe0, 0x00, 0x38, 0x70, 0x00, 0x18, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00};
    display.clearDisplay();
    display.drawBitmap(8, 2, battery, 64, 28, WHITE);
    if (ADS1115_alive){
        unsigned int batteryLevel = getBatteryLevel();
        if (batteryCharging()){
            display.drawBitmap(26, 10, lightning, 20, 10, 1);
        } else {
            display.fillRect(12, 6, 51*batteryLevel/100, 20, SSD1306_INVERSE);
        }
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(75, 10);
        display.print(batteryLevel);
        display.print("%");
    } else {
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setCursor(75, 10);
        display.print("?? %");
    }
    display.display();
}

void displayWiFi(){
    static PROGMEM const unsigned char WiFiStrong[] = 
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x01, 
	0xff, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xc7, 0xff, 0x80, 0x0f, 0xf0, 0x00, 0x3f, 0xc0, 0x1f, 0xc0, 
	0x00, 0x0f, 0xf0, 0x3f, 0x00, 0x00, 0x03, 0xf8, 0x7c, 0x00, 0x00, 0x00, 0xfc, 0xf8, 0x07, 0xff, 
	0x80, 0x7c, 0x70, 0x1f, 0xff, 0xf0, 0x3c, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0xff, 0x03, 0xfe, 
	0x00, 0x03, 0xf8, 0x00, 0x7f, 0x00, 0x03, 0xe0, 0x00, 0x1f, 0x80, 0x03, 0xc0, 0x00, 0x0f, 0x80, 
	0x03, 0x80, 0x30, 0x03, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 
	0x0f, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xc7, 0xe0, 0x00, 0x00, 0x1e, 0x01, 0xe0, 0x00, 0x00, 0x0c, 
	0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x78, 
	0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00   };
    static PROGMEM const unsigned char WiFiNormal[] = 
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 
	0x80, 0x00, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0xff, 0x03, 0xfc, 
	0x00, 0x01, 0xf8, 0x00, 0x7f, 0x00, 0x03, 0xe0, 0x00, 0x1f, 0x80, 0x07, 0xc0, 0x00, 0x0f, 0x80, 
	0x03, 0x80, 0x30, 0x07, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 
	0x0f, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xc7, 0xe0, 0x00, 0x00, 0x1e, 0x01, 0xe0, 0x00, 0x00, 0x0c, 
	0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x78, 
	0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00  };
    static PROGMEM const unsigned char WiFiWeak[] = 
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 
	0x0f, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xc7, 0xe0, 0x00, 0x00, 0x1e, 0x01, 0xe0, 0x00, 0x00, 0x0c, 
	0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x78, 
	0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00  };
    display.clearDisplay();
    int signalStrength = WiFi.RSSI();
    if (signalStrength > -55){
        display.drawBitmap(45, 2, WiFiStrong, 38, 28, 1);
    } else if (signalStrength < -75){
        display.drawBitmap(45, 2, WiFiWeak, 38, 28, 1);
    } else {
        display.drawBitmap(45, 2, WiFiNormal, 38, 28, 1);
    }
    display.display();
}

bool ManualFactoryReset(){
    int start = millis();
    bool manual = true;
    printClearOLED(processText("Cannot connect to server. Tap within 10 seconds to proceed manual factory reset").c_str(), WHITE, 1);
    while (getTouchValue(TOUCH) > threshold_touch){
        if (millis() - start > 10000){
            manual = false;
            break;
        }
    }
    return manual;
}
