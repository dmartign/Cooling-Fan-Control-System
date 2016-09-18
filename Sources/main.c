// ECE 470 - FINAL PROJECT - DC-MOTOR FAN CONTROL SYSTEM
// Daniela Martignani
// John Ybarra


#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include "util.h"
#include "sci0.h"
#include "ATD.h"
#include "interrupts_RTI.h"
#include <stdio.h>    // needed for some I/O functions

/////////////////// GLOBAL VARIABLES DECLARATION & CONSTANTS DECLARATION SECTION
int stopped, controlled;   // flags for fan status

// delay generation
#define DS 5
#define COUNT 18750

// SCI
#define UPDATE_RATE   5
#define UPDATE_RATE2   10
#define REFRESH_RATE  10
int update_time, update_time2;
int refresh_time, refresh_time2;
volatile int update, update2, refresh, refresh2;

// motor
#define MAX_SPEED 255
#define MIN_SPEED 0
int theSpeed;
#define FORWARD 1

// Input Capture and rpm measurement - reed switch
unsigned int first_capture, second_capture, 
             edge1, edge2, next_edge, 
             bouncing_time, diff, rpm, captured,period; 
long int speed_rps, rps;

// SCI module
char names[35] = "John Ybarra, Daniela Martignani";  // names to be displayed
char buf1[100];       // to hold greeting to be displayed
char buf_mode[100];   // to print the instructions in the screen
char buf_mode2[100];  // ''  ''    ''   ''         ''  ''   ''
char buf_mode3[100];  // ''  ''    ''   ''         ''  ''   ''
char buf_rpm[100];    // ''  ''    ''   ''         ''  ''   ''
char buf_status[100]; // ''  ''    ''   ''         ''  ''   ''



///////////////////// FUNCTION DEFINITIONS
/*-----------------------------------------------------------------------
              SCI0_ISR: controls fan in manual mode based on user input of 
                        up/down keyboard arrows 
-------------------------------------------------------------------------*/

interrupt 20 SCI0_ISR(){
     
     char c = SCI0SR1;
     c = SCI0DRL;           //both reads will clr the flag
      
      if (fan_state == ON){
        
        if(mode == MANUAL_MODE){ 
             
            if(c == 65){            //cursor up code is 65 
                theSpeed += DS; 
                if(theSpeed >= MAX_SPEED){
                    theSpeed = MAX_SPEED;
                }
                                       
            }else if(c == 66){        // cursor down code is 66
                theSpeed -= DS; 
                if(theSpeed <= MIN_SPEED){
                    theSpeed = MIN_SPEED;
                }
            }
        
        }
      }
        
  }


/*-----------------------------------------------------------------------
              intH_enable: for causing interrupt of S1 (ON/OFF fan status)
-------------------------------------------------------------------------*/

void intH_enable(){  //PORT H Interrupt Enable 
    PIEH = 0x01;     // PIEH --> Port H Interrupt Enable Register --> PIEH0 = 1 --> Enabled  
    PIFH = 0x01;     // PIFH --> Port H Interrupt Flag Register   --> PIFH0 = 1 --> Cleared 
}




/*-----------------------------------------------------------------------
              intH_ISR: for performing actions in S1 (ON/OFF fan status)
-------------------------------------------------------------------------*/

interrupt 25 void intH_ISR(){   // INTERRUPT SERVICE ROUTINE FOR ON/OFF 

    PIFH = 0x01;   // write 1 to clear flag
    
    // FOR ON/OFF BTN
    if(button_state1 == NOT_PRESSED){
      
        button_state1 = PRESSED;
        
        task4_time1 =  TASK4_RATE;
        task4_active1 = 1;
        
        // changes state of the fan 
        if(fan_state == ON){
        
            fan_state = OFF; 
            theSpeed = MIN_SPEED;
            
        }else{
        
            fan_state = ON;
            theSpeed = (MAX_SPEED/2);
            
        }
               
    }
    
}



/*-----------------------------------------------------------------------
              OC5_ISR: for creating 500ms delay for temp sensor readings. OC5
-------------------------------------------------------------------------*/

interrupt 13 OC5_ISR(){

    TC5 += COUNT;   //clear TFLG1 bit 5 and extend for another 100 ms
    if(update_time > 0){
        update_time--;
        if(update_time == 0){
            update = 1; 
        }
    }
      
}



/*-----------------------------------------------------------------------
              OC3_ISR: for creating 1s delay for refresh temp readings on the screen
                       OC3
-------------------------------------------------------------------------*/
interrupt 11 OC3_ISR(){

    TC3 += COUNT;   //clear TFLG1 bit 3 and extend for another 1000 ms
    
    if(refresh_time2 > 0){
        refresh_time2--; 
        if(refresh_time2 == 0){
            refresh2 = 1;
           
        }
    }
    
}


/*-----------------------------------------------------------------------
              OC5_OC3_init: initializes the interrupt conditions for the 
                            OC3, OC5, IC0 
-------------------------------------------------------------------------*/
void OC5_OC3_init(){

    TSCR1 = 0x90;   // enable Timer with Fast mode
    TSCR2 = 7;      // 24Mhz/128 
    
    TIOS = (1<<5) | (1<<3) ;   //enable OC5, OC3, OC1 and IC0
 
    TCTL4 = 0x01;     // input capture on rising edges only for channel 0 FOR EXTRA CREDIT PART
     
    TC5 = TCNT + COUNT; //clear TFLG1 bit 5
    TC3 = TCNT + COUNT; //clear TFLG1 bit 3
      
    TIE = (1<<5) | (1<<3) | (1<<0);   // enables interrupts for OC5, OC3, OC1 and IC0
       
}



/*-----------------------------------------------------------------------
              IC0_ISR --> EXTRA CREDIT PART: uses magnetic reed switch to 
                          measure rpm of fan
-------------------------------------------------------------------------*/
unsigned value = 10 * 1875/10; // after 1875 ticks (10ms*187.5Kh = 1875 ticks) bouncing should be finished

interrupt 8 IC0_ISR(){

    unsigned e = TC0;      // e clears TC0 to begin with and then receives every value read to TC0
     
    if (first_capture == 1){
    
        edge1 = e;
        first_capture = 0;         // first edge captured
        second_capture = 1;        // waiting to capture second edge
    
   
    } else if (second_capture == 1){       // evaluating the following edges captured
    
        next_edge = e;                     // wait for approx 10us of bouncing edges reception
        bouncing_time = next_edge - edge1;   
        
        if (bouncing_time > value){        
            edge2 = next_edge;             // the next coming edge should be safe to be captured 
            second_capture = 0;
            captured = 1;
        }
     
    }
  
   
}



/*-----------------------------------------------------------------------
              motor_stop
-------------------------------------------------------------------------*/
void motor_stop(){

   PWM0_duty(0);

}


/*-----------------------------------------------------------------------
              motor_forward: for moving the motor forward (only one direction)
-------------------------------------------------------------------------*/
void motor_forward(){

    motor_stop();
    delay_ms(1000);
    DDRB = 0xFF;
    PORTB = 2;  

}


/*-----------------------------------------------------------------------
              motor_speed
-------------------------------------------------------------------------*/
void motor_speed(int s){

  PWM0_duty(s);  

}


//////////////////////  MAIN FUNCTION
/*-----------------------------------------------------------------------
              main
-------------------------------------------------------------------------*/  
void main(void) {

  // general display and ATD variables
  int value;
  int volts;
  int temp;
  char buf[100];  // to hold data to be received
  char buf_name[35];   // to hold names to be displayed
  
  // controls fan status - flag variables, set in interrupt
  stopped = 0;
  controlled = 0;
  
  // for fan
  theSpeed = MAX_SPEED/2;   // speed =
  
  // for PLL clk
  ECLK_24Mhz();
  
  // for SCI 
  SCI0_init(9600);
  SCI0_enableRX_int();
  
  // for motor/fan
  motor_forward();
  motor_speed(theSpeed);
  
  PWM0_100Hz();         

  // for SCI
  ATD0_init();
  OC5_OC3_init();
       // initial states
  update_time = UPDATE_RATE;
  update_time2 = UPDATE_RATE;
  refresh_time = REFRESH_RATE;
  
  update  = 1;
  refresh = 1;  
  update2  = 1;
  refresh2 = 1;
  
  // for rpm
  first_capture = 1;
  second_capture = 0;
  captured = 0;
  
  // for pushbuttons
  DDRH = 0x00;    // enable both for input
  DDRJ = 0x00;
  
  RTI_enable();
  
  intH_enable();
  
  intJ_enable();
  
  rgb_enable();
  
  task4_active1 = 1;
  task5_active1 = 1;
  
  fan_state = ON;   // default to start the fan should be ON
  mode = AUTO_MODE;  // default to start the mode of operation is automatic (user doesn't control fan)
  
  rgb_color();      // shows LED in manual and auto mode
        // initial states
  button_state1 = NOT_PRESSED;
  button_state2 = NOT_PRESSED;
  
  counter_state = 0;
  counter_state2 = 0;
 
  
  
  // enables interrupts globally
  asm{
    cli
  }
  
 
  // prints names in the screen at the beginning of the program
  // then formats more output and instructions for the user
   sprintf(buf1, "\t*************ECE 470 FINAL PROJECT - Fall 2013*************\n       ");   // output names formatted as a string 
   SCI0_putline(buf1);
   sprintf(buf_name, "\r\t\t %s \n                   ", names);   // output names formatted as a string 
   SCI0_putline(buf_name); 
   
   
   sprintf(buf_mode, "\n\rModes of operation - using S2 button:                    \n                          ");
   SCI0_putline(buf_mode);
   sprintf(buf_mode2, "\rRED: Auto Mode - fan speed adjusts to the room temperature \n\r      (shuts down if temp < 18ºC, or runs at max speed if temp reaches 25ºC)\n                            ");
   SCI0_putline(buf_mode2); 
   sprintf(buf_mode3, "\rGREEN: Manual Mode - change speed of fan by up/down arrows\n                            ");   // output names formatted as a string 
   SCI0_putline(buf_mode3);
    
   sprintf(buf_status, "\n\rControl fan ON/OFF - using S1 button                    \n\n                          ");
   SCI0_putline(buf_status);
   
     
 
  for(;;){
  
    // makes update of temperature readings
    if(update && fan_state == ON){
      
        value =  ATD0_convertFrom(5); // channel 5 used, "value" holds digital conversion from analog voltage
        volts = (value * 49L)/10;     // step size is 4.9V
        temp = volts / 10;            // temp is in Celsius
        update = 0;
        update_time = UPDATE_RATE;
        
        /*
        if(temp in range1) speed1
        else if(tamp in range2) speed2
        else turn off
        */
        
        if (mode == AUTO_MODE){
          
            if (temp < 18){
                theSpeed = 0;           // min speed
            } else if (temp == 18){
                theSpeed = 32;  
            } else if (temp == 19){
                theSpeed = 64;  
            } else if (temp == 20){
                theSpeed = 96;    
            } else if (temp == 21){
                theSpeed = 128;  
            } else if (temp == 22){
                theSpeed = 160;  
            } else if (temp == 23){
                theSpeed = 192;  
            } else if (temp == 24){
                theSpeed = 224;  
            } else if (temp >= 25){
                theSpeed = MAX_SPEED; // 255 is the highest speed                     
            }
            
       }
        
    }//update
   
    
    // refresh of values on the screen
    if(refresh2 && fan_state == ON){
           sprintf(buf, "\rATD value = %4d, volts = %5d mV, temp = %2d ºC, duty = %3d, ", value, volts, temp, theSpeed);
           SCI0_putline(buf);
          
           if (captured == 1){         // once the 2 edges are captured, do the period/rpm calculation
    
                diff = edge2 - edge1;
                period = diff*10L/1875;
                rpm = 60000L/period;
                
                sprintf(buf_rpm, "fan period =  %3d ms, rpm =  %3d            ", period, rpm);
                SCI0_putline(buf_rpm);
                
                first_capture = 1;     // sets conditions to repeate readings again
                second_capture = 0;
                captured = 0;
                     
           }
    
          refresh2 = 0;
          refresh_time2 = REFRESH_RATE;
    }
    
   
     
    motor_speed(theSpeed);  
      
        
    // to perform action after the switches (s1-PM6 or s2-PM7) are pushed
    if (task4_active1 && 0 == task4_time1){
        task4_time1 = TASK4_RATE;   // reschedule my task4 (changes mode AUTO_MODE/MANUAL_MODE)
        task4();  
    }
    
    if (task5_active1 && 0 == task5_time1){
        task5_time1 = TASK5_RATE;   // reschedule my task5 (changes fan_state ON/OFF)
        task5();   
    }  
      
      
     // displays message on screen when fan is manually turned off 
    if(fan_state == ON){
        
       stopped = 0;
   
    }else{
     
       if(stopped == 0){
           sprintf(buf, "\rSystem has been shut down                                                                          ");
           SCI0_putline(buf);
           stopped = 1;
       }
     
    }
     
  
  }// ends for(;;)
   
  
}  // ends main
