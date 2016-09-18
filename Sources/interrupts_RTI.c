#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "util.h"
#include "interrupts_RTI.h"                                                                    
                                                                     
                                                                     
                                             
////////////////// real time interrupt routines ////////// 
void RTI_enable(){    //RTI circuit generates hardware interrupts periodically when enabled 

  CRGINT = 0x80;      // Clock and Reset Generation Interrupt Enable Register 
                      // 0x80 --> bit7 of CRGINT --> RTIE (Real-Time Interrupt Enable bit)
                      // bit7 = 1 --> Interrupt requests from RTI are enabled
  RTICTL = 0x60;      // CRG RTI control register sets the rate that RTI occurs
  // ****was 0x40, since 0x60 is for 24MHz, and log result of 2^15 approx
                      // FROM  1ms = x/8MHz --> x = 8000
                      // THEN 2^i = 8000  --> i = log_2(8000) = 12.9 =13 --> 2^13
                      // use table 6.4 on page 287  for 2^13
                      // 0x40 --> RTR[7] = 0 , RTR[6:4] = 100 , RTR[3:0] = 0000 --> 2^13 = 8192 
} // end RTI


#define ON   1
#define OFF  2

#define AUTO_MODE   1
#define MANUAL_MODE  2

#define PRESSED 1
#define NOT_PRESSED 2 
#define BOUNCING 3

int fan_state, mode; //controlled;                   	// declare integer for ON/OFF and AUTO_MODE/MANUAL_MODE respecively
int button_state1, button_state2; 			// declare two integers for two push buttons 


void intJ_enable(){  //PORT H Interrupt Enable 
    PIEJ = 0x01;     // PIEJ --> Port J Interrupt Enable Register --> PIEJ0 = 1 --> Enabled  
    PIFJ = 0x01;     // PIFJ --> Port J Interrupt Flag Register   --> PIFJ0 = 1 --> Cleared 
}

/////////////////////////// task 4  ///////////////

#define TASK4_RATE  2
#define TASK5_RATE  2
int task4_time1, task5_time1;
int task4_active1, task5_active1;
int counter_state, counter_state2;



//debounce both push buttons
//this task will be running periodically every 2ms to sample and debounce my switch
//once the switch is up, it deactivates itself
void task4(){
      
      // ================ FOR DEBOUNCING ON/OFF BUTTON
     
      counter_state <<= 1; 
      
      counter_state |= (PTM>>7) & 1;       // S2 --> PM7 
      
      if(counter_state == 0){
        
          button_state1 = PRESSED;
        
      }else if(counter_state == 0xFFFF){
          
          button_state1 = NOT_PRESSED;
	      counter_state = 0;
          task4_active1 = 0; 
        
      }else{
        
          button_state1 = BOUNCING;
      
      }
     
}


void task5(){
      
      // ================ FOR DEBOUNCING AUTO/MANUAL MODE
     
      counter_state2 <<= 1 ;
      
      counter_state2 |= (PTM>>6) & 1;       // S1 --> PM6
      
      if(counter_state2 == 0){
        
          button_state2 = PRESSED;
        
      }else if(counter_state2 == 0xFFFF){
          
          button_state2 = NOT_PRESSED;
          counter_state2 = 0 ;
          task5_active1 = 0; 
        
      }else{
        
          button_state2 = BOUNCING;
      
      }
     
}
  

 //activate or deactivate task1,task2, and task3 in an alternative way
 interrupt 24 void intJ_ISR(){     // INTERRUPT SERVICE ROUTINE FOR AUTO/MANUAL MODE
 
    PIFJ = 0x01;   // write 1 to clear flag!
    //PIFH = 0x01;
     
    if(button_state2 == NOT_PRESSED){
        
        task5_time1 =  TASK5_RATE;
        task5_active1 = 1;  // DEBOUNCE FOR RESET BUTTON IS ACTIVE   
     
        if(mode == AUTO_MODE){
            mode = MANUAL_MODE;
            rgb_color();              
        }else{
            mode = AUTO_MODE;
            rgb_color();
        }
      
	  }
    button_state2 = PRESSED;
    
}                // THIS SETS A DEFAULT REGARDLESS OF THE STATE OF THE COUNTER WHEN RESET IS TRIGGERED 


interrupt 7 void RTI_ISR(){
      CRGFLG = 0x80;        //Clear interrupt flag
        
      if (task4_active1 && task4_time1 > 0 ) //tick down task
          task4_time1 --;
      
      if (task5_active1 && task5_time1 > 0 ) //tick down task
          task5_time1 --;
      
}



//////////////// rgb function ///////////////////

void rgb_enable(){   //definition
  	DDRP = 0xFF;
}

void rgb_color(){
      
    if(mode == AUTO_MODE){
      PTP = 0x10;            //red
    }else if(mode == MANUAL_MODE){
      PTP = 0x40;       //green
     
    }
    
    
}



