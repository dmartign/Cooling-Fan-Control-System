#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */





void ECLK_24Mhz(){
    SYNR =2;
    REFDV= 0;
    CLKSEL=0; //use OSCLK
    PLLCTL= (1<<5) | (1<<6); //turn on PLL and use AUTO
    while (  !(CRGFLG & (1<<3)) );
    CLKSEL = 1<<7;   
}






void delay_1ms(){
  
   asm{
        PSHX                 
        LDX #7995           
        
   WAIT:
        DBNE X, WAIT        
        
        PULX                
   }
}

void delay_ms(int millis){
    while(millis--)
        delay_1ms();     
     
}






