#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */

//i need 100 Hz (10ms)
//==> PWMCNT clock /255 = 100Hz ==> PWMCNT clock = 25.5 Khz
//24 MHz/(P*2*N) = 25.5 KHz ==> PN = 471
//Try P=8 --> N = 59 ==> PWMCNT clock = 25.4 KHz nice.

void PWM0_100Hz(){
    PWMCTL = 0;  //separate 8-bit channels
    PWMPOL=1;  //start high
    PWMCAE=0; //left align
    PWME =1;
    PWMPRCLK=3;   //clock A = 24/8 = 3 MHz
    PWMSCLA =59;    //clock SA = 3000Khz/(2*59) = 25.4 KHz
    PWMCLK=1;
    PWMPER0=255;  // PP0 freq = 25.4Khz/255 = 99.7 Hz  (~10 ms)
    PWMDTY0=128;    
}




void PWM0_10KHz(){
    PWMCTL = 0;  //separate 8-bit channels
    PWMPOL=1;  //start high
    PWMCAE=0; //left align
    PWME =1;
    PWMPRCLK=0;   //clock A = 24/1 = 24 MHz
    PWMSCLA =5;    //clock SA = 24Mhz/(2*%) = 2.4 MHz
    PWMCLK=1;
    PWMPER0=255;  // PP0 freq = 2.4 Mhz/255 =9.4 KHz 
    PWMDTY0=128;    
}



void PWM0_duty(int d){
    PWMDTY0=d;    
}