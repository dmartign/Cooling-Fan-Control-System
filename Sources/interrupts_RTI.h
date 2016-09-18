                                                              
                                                                     
                                                                     
// function declarations from "interrupts_RTI.c"                                             
  
void RTI_enable();     

void intJ_enable();   
  
void task4();

void task5();

interrupt 24 void intJ_ISR();

interrupt 7 void RTI_ISR();

void rgb_enable();

void rgb_color();


// variables from "interrupts_RTI.c" defined "extern" so they can be used in "main.c" 
extern int task4_active1, task5_active1;
extern int counter_state, counter_state2;

#define TASK4_RATE  2  
#define TASK5_RATE  2

extern int task4_time1, task5_time1;
extern int task4_active1, task5_active1;
extern int counter_state, counter_state2;

#define ON   1
#define OFF  2

#define AUTO_MODE   1
#define MANUAL_MODE  2

#define PRESSED 1
#define NOT_PRESSED 2 
#define BOUNCING 3

extern int fan_state, mode; //controlled;      	// declare integer for ON/OFF and AUTO_MODE/MANUAL_MODE respectively
extern int button_state1, button_state2; 			// declare two integers for two push buttons 



  
