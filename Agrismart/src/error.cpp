#include <esp32-hal-ledc.h>
extern int ERROR;
int solid_frequency = 1000;   //solid 
int blink_frequency = 1;   // low

void OLED_error(){
    ledcSetup(0, solid_frequency, 8);
    ledcAttachPin(ERROR, 0);
    ledcWrite(0, 99);
    
}


void server_error(){
    ledcDetachPin(ERROR);
    ledcSetup(1, blink_frequency, 8);
    ledcAttachPin(ERROR, 1);
    ledcWrite(1, 99);
}

// void TOF_error(){
//     ledcDetachPin(ERROR);
//     ledcSetup(2, low_frequency, 8);
//     ledcAttachPin(ERROR, 2);
//     ledcWrite(2, 1);
// }