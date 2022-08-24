#include <esp32-hal-ledc.h>
extern int ERROR;
int high_frequency = 1000;   //solid 
int low_frequency = 1;   //fast

void OLED_error(){
    ledcDetachPin(ERROR);
    ledcSetup(0, high_frequency, 8);
    ledcAttachPin(ERROR, 0);
    ledcWrite(0, 99);
    
}

// void TempHum_error(){
//     ledcDetachPin(ERROR);
//     ledcSetup(1, low_frequency, 8);
//     ledcAttachPin(ERROR, 1);
//     ledcWrite(1, 99);
// }

// void TOF_error(){
//     ledcDetachPin(ERROR);
//     ledcSetup(2, low_frequency, 8);
//     ledcAttachPin(ERROR, 2);
//     ledcWrite(2, 1);
// }