#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "sci0.h"


#define TDRE  (1<<7)


void SCI0_init(long br){       // br is baud rate
    
   int SBR = (int)(24000000/(16*br));   
   SCI0BD = SBR;
   SCI0CR1 = 0;  //8NP, 8 data bits, parity disabled, even parity
   SCI0CR2 = (1<<2) | (1<<3) ;  // (1<<2) --> RE=1 (receive enable) and (1<<3) --> TE=1 (transmit enable)
     
}


void SCI0_enableRX_int(){

  SCI0CR2 |= (1<<5); // (1<<5) --> RIE=1, RDRF and OR interrupt enabled (receiver full interrupt enable bit) 

}


void SCI0_putchar(char c){  // transmits data

   while (!(SCI0SR1 & TDRE)) {//SCI0SR1, bit7=TDRE (if high (1)) transmit data reg is empty (done transferring)  
   }                          //TDRE, bit7(TIE) of SCIxCR2 (when=1 TDRE interrupt enabled)
                              // when both values are 1, loop condition will break, and transmission is completed
   SCI0DRL = c; 
  
}


unsigned char SCI0_getchar(){  // receives data

    char c;
  
    while (!(SCI0SR1 & (1<<5))) { // (1<<5) --> bit5 of SCIxSR1=RDRF (receiver data reg full flag)    
    }                             //            if 1, SCIxDR is full (all received)
                                  // when both values are 1, done receiving and all contents are stored in 
    c = SCI0DRL ;                 // variable c
    
    return c;
  
}


unsigned char SCI0_getcharNB(){ // non-blocking function, same as the one above (SCI0_getchar())

    char c;
  
    if((SCI0SR1 & (1<<5))) {
        return SCI0DRL;
    }
    
    return 0;  
}



void SCI0_putline(char buf[]){ // data stored in buf[], so iterating through each element to display it
  
  int i;
  
  for(i = 0; buf [i] != '\0'; i++){
      SCI0_putchar(buf[i]);  // sending "c" to the above function to transmit each bit serially
  } 
  
}


int SCI0_getline(char buf[]){
    
    char c;
    int i;     
    i = 0;
    c = SCI0_getchar();   // when receiving data serially, storing it ("c") into buf[]
                          // so the whole message can be stored sequentially 
    while (c != '\r' && c !='\n'){                     
         buf[i] = c;
         i++;
         c = SCI0_getchar(); 
        
    }
    
    buf[i] = 0;
      
     return i;

 }



